import srl, srl_event_detector
from pprint import pprint
import time, sys

def wait_for_data( connection, max_timeout = 5 ):
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

# Wait until task is complete.
def wait_on_task( message ):
    if 'task-id' in message:
        task_id = message['task-id']
    else: return None

    while True:
        c.send(srl_event_detector.TaskProgressMessageFactory.\
                generate_progress_request(task_id))
        r = srl.InterfaceMessage().decode(wait_for_data(c))
        #pprint(r.fields)
        if 'progress' not in r:
            break
        elif r.fields['body']['progress'] >= 1:
            break
        else: time.sleep(5)
    return r

# Establish a connection to the server.
if len(sys.argv) == 2:
    port = int(sys.argv[1])
else: port = 12345
print('Connecting to port %d...' % port)
c = srl.TCPConnection('127.0.0.1:%d' % port)

# Run a query.
print('Getting superbowl query results...')
c.send(srl_event_detector.CheapEventReportRequestFactory.generate(
    'NORTH_JERSEY_NEW_JERSEY', [], 'Jan/05/2014', 'Mar/02/2014', baseline_location='NEW_YORK_NEW_YORK'))
r = srl.InterfaceMessage().decode(wait_for_data(c))
pprint(r.fields)

# Wait until analysis completes.
r = wait_on_task(r)

# Print results.
print('Done. Here are the results:')
for res in r['results']:
    print(res)

# Shutdown server.
print('Shutting down server.')
c.send(srl.ShutdownMessageFactory.generate())
print(wait_for_data(c))
