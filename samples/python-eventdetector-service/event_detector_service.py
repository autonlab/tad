from multiprocessing import Process, Manager, Queue, Lock
import time
from event_detector import EventDetector
import srl, srl_event_detector

def safe_print( l, m ):
    l.acquire()
    print(m)
    l.release()

def worker_process( i, t, r, s, pl ):
    safe_print(pl, '  - Worker process %d started.' % i)

    iters = 0
    while True:
        task                = t.get()

        path                = s[0]
        additional_phones   = s[1]
        phone2cluster       = s[2]

        iters               += 1
        if task == ['exit']:
            break

        elif task[1] == 'init':
            r.put([task[0], 0])

            additional_phones, phone2cluster = \
                    EventDetector.create_phone_clusters(path)

            s[1] = additional_phones
            s[2] = phone2cluster

            r.put([task[0], 1, None])

        elif task[1] == 'process':
            r.put([task[0], 0])

            result = EventDetector.cheap_event_report(
                    path, additional_phones, phone2cluster,
                    task[2], task[3], task[4], task[5])

            r.put([task[0], 1, result])
            safe_print(pl, '  - Worker %d: Finished processing task ''%s''' % (i, task[1]))

        else:
            safe_print(pl, '  - Worker %d: Error, unknown task ''%s''' % (i, task[0]))

    safe_print(pl, '  - Worker process %d exited.' % i)

def wait_for_data( connection, max_timeout = 1 ):
    time_waited = 0
    while time_waited < max_timeout:
        try:
            response = connection.receive()
        except:
            return ''

        if not response:
            time.sleep(0.1)
            time_waited += 0.1
        else: break

    return response

if __name__ == '__main__':
    global killed
    killed = False

    print('Master thread started.')

    # Establish connection with server.
    connection = srl.TCPConnection(':12345')

    # Register available services.
    connection.send(srl.RegisterServiceMessageFactory.generate(
        srl_event_detector.ServiceName, ['Progress', 'Init', 'CheapEventReport']))
    response = wait_for_data(connection, 5)
    if not response: raise Exception('Could not register services with the server!')

    manager = Manager()

    # Initialize shared objects.
    path = 'snapshot/'

    t = Queue()             # Task queue
    r = Queue()             # Result queue
    s = manager.list()
    s.append(path)          # Path
    s.append({})            # additional_phones
    s.append({})            # phone2cluster
    pl = Lock()

    # Initialize child worker pool.
    proc_count = 2
    procs = [None]*proc_count;
    for i in xrange(0, proc_count):
        procs[i] = Process(target=worker_process, args=(i, t, r, s, pl))
        procs[i].start()

    # Wait for service requests.
    Uninitialized   = 0
    Initializing    = 1
    Initialized     = 2

    state           = Uninitialized
    next_task_id    = 100
    next_noop       = 0

    tasks = {}
    while not killed:
        # Send noops to keep us alive!
        if time.time() > next_noop:
            next_noop = time.time() + 60*2
            connection.send(srl.NoOpMessageFactory.generate())
            wait_for_data(connection)

        # Handle any updates from the tasks.
        while not r.empty():
            update = r.get()
            tasks[update[0]]['progress'] = update[1]
            if update[1] >= 1:
                tasks[update[0]]['results'] = update[2]
                if update[2] == None:
                    state = Initialized

        # Handle incoming messages.
        raw_message = wait_for_data(connection)
        if raw_message:
            message = srl.InterfaceMessage().decode(raw_message)

            # Handle Builtin messages.
            if message.get_module() == 'Builtin':
                if message.get_service() == 'Disconnect':
                    print('Disconnect received: ' + message['reason'])
                    break
                elif message.get_service() == 'Shutdown':
                    print('Server is shutting down, so will this service.')
                    break

            # Handle service calls.
            elif message.get_module() == srl_event_detector.ServiceName:
                if message.get_service() == 'Progress':
                    print(' * progress')
                    if message['task-id'] in tasks:
                        if tasks[message['task-id']]['progress'] < 0:
                            connection.send(srl_event_detector.TaskProgressMessageFactory.generate(
                                message, 0, 'Still queued.'))
                        else:
                            if (tasks[message['task-id']]['progress'] >= 1) and \
                               (tasks[message['task-id']]['results'] != None):
                                connection.send(srl_event_detector.CheapEventReportResponseFactory.generate(
                                    message, tasks[message['task-id']]['results']))
                            else:
                                connection.send(srl_event_detector.TaskProgressMessageFactory.generate(
                                    message, tasks[message['task-id']]['progress'], 'Started.'))
                    else:
                        connection.send(srl.ErrorMessageFactory.generate(
                            message, 'Unknown task.'))

                elif message.get_service() == 'Init':
                    print(' * init')
                    if state == Uninitialized:
                        tasks[next_task_id] = {'progress': -1, 'results': None}
                        t.put([next_task_id, 'init'])
                        connection.send(srl_event_detector.TaskProgressMessageFactory.generate(
                            message, 0, 'Initialization started.', next_task_id))
                        next_task_id += 1
                        state = Initializing
                    elif state == Initializing:
                        connection.send(srl.ErrorMessageFactory.generate(
                            message, 'Already initializing.'))
                    elif state == Initialized:
                        connection.send(srl.StatusMessageFactory.generate(
                            message, 'Initialized.'))
                    else:
                        connection.send(srl.ErrorMessageFactory.generate(
                            message, 'Unknown state!'))

                elif message.get_service() == 'CheapEventReport':
                    print(' * report')
                    if state != Initialized:
                        connection.send(srl.ErrorMessageFactory.generate(
                            message, 'Not yet initialized.'))
                    else:
                        task = srl_event_detector.CheapEventReportRequestFactory.parse(message)
                        tasks[next_task_id] = {'progress': -1, 'results': None}
                        t.put([
                            next_task_id, 'process',
                            task['target-location'], task['keylist'],
                            task['analysis-start-date'], task['analysis-end-date']])
                        connection.send(srl_event_detector.TaskProgressMessageFactory.generate(
                            message, 0, 'Event report request queued.', next_task_id))
                        next_task_id += 1

                else: print('Unknown service: %s' % message.get_service())

            # Unknown module?
            else: print('Unknown module: %s' % message.get_module())

    # Killed?
    if killed:
        print('Service was killed.')

    # Kill all workers.
    print('Destroying workers...')
    for i in xrange(0, proc_count):
        t.put(['exit'])

    # Join.
    for proc in procs:
        proc.join()

    # Done.
    print('Master thread finished.')
