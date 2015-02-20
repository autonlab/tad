from multiprocessing import Process, Manager, Queue, Lock
import time
import classifier_services as cs
from datetime import datetime

def safe_print( l, m ):
    l.acquire()
    print(m)
    l.release()

def worker_process( i, q, s, pl ):
    safe_print(pl, '  - Worker process %d started.' % i)

    iters = 0
    while True:
        task                = q.get()

        path                = s[0]
        additional_phones   = s[1]
        phone2cluster       = s[2]

        iters               += 1
        if task == ['exit']:
            break

        elif task == ['init']:
            additional_phones, phone2cluster = \
                    cs.EventDetector.create_phone_clusters(path)
            s[1] = additional_phones
            s[2] = phone2cluster

        elif task[0] == 'process':
            result = cs.EventDetector.cheap_event_report(
                    path, additional_phones, phone2cluster,
                    task[2], task[3], task[4], task[5])
            with open(task[1], 'w') as f:
                for x in result:
                    f.write(x[0].strftime('%m/%d/%Y') + ',' + ','.join([str(y) for y in x[1:]]) + "\n")
            safe_print(pl, '  - Worker %d: Finished processing task ''%s''' % (i, task[1]))

        else:
            safe_print(pl, '  - Worker %d: Error, unknown task ''%s''' % (i, task[0]))

    safe_print(pl, '  - Worker process %d exited.' % i)

if __name__ == '__main__':
    print('Master thread started.')

    manager = Manager()

    # Initialize shared objects.
    path = 'snapshot/'

    q = Queue()
    s = manager.list()
    s.append(path)          # Path
    s.append({})            # additional_phones
    s.append({})            # phone2cluster
    pl = Lock()

    # Initialize child worker pool.
    proc_count = 2
    procs = [None]*proc_count;
    for i in xrange(0, proc_count):
        procs[i] = Process(target=worker_process, args=(i, q, s, pl))
        procs[i].start()

    # Ask for initialization, and wait until complete.
    safe_print(pl, 'Waiting on initialization...')
    q.put(['init'])
    while (len(s[1]) == 0) or (len(s[2]) == 0):
        time.sleep(1)

    # Handle the two classifications.
    safe_print(pl, 'Initialization finished. Sending processing tasks...')
    q.put(['process',
        'mp_superbowl.csv',
        'NORTH_JERSEY_NEW_JERSEY', [],
        datetime.strptime('Jan/05/2014','%b/%d/%Y').date(),
        datetime.strptime('Mar/02/2014','%b/%d/%Y').date()])
    q.put(['process',
        'mp_atlanta.csv',
        'ATLANTA_GEORGIA', [],
        datetime.strptime('Feb/11/2014','%b/%d/%Y').date(),
        datetime.strptime('Feb/11/2015','%b/%d/%Y').date()])

    # Kill all workers.
    for i in xrange(0, proc_count):
        q.put(['exit'])

    # Join.
    for proc in procs:
        proc.join()

    # Done.
    print('Master thread finished.')
