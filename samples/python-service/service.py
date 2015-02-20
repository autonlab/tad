import time
import srl
from pprint import pprint

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

# Establish connection with the server.
connection = srl.TCPConnection('127.0.0.1:12345')

# Register available services.
connection.send(srl.RegisterServiceMessageFactory.generate('PyService', ['add', 'subtract']))
response = wait_for_data(connection)
if not response: raise Exception('Could not register services with the server.')
pprint(srl.InterfaceMessage().decode(response).fields)

# Wait for service requests.
next_noop = 0
while True:
    if time.time() > next_noop:
        print('Sending NoOp')
        next_noop = time.time() + 60*1
        print(connection.send(srl.NoOpMessageFactory.generate()))
        print(wait_for_data(connection))

    raw_message = wait_for_data(connection)
    if raw_message:
        message = srl.InterfaceMessage().decode(raw_message)
        pprint(message.fields)

        # Handle Builtin messages.
        if message.get_module() == 'Builtin':
            if message.get_service() == 'Disconnect':
                print('Disconnect received.')
                break
            elif message.get_service() == 'Shutdown':
                print('Server has shutdown. Exiting service.')
                break

        # Handle service calls.
        elif message.get_module() == 'PyService':
            if message.get_service() == 'add':
                response = srl.InterfaceMessage('PyService', 'add', message.get_client_id())
                response['result'] = int(message['a']) + int(message['b'])
                print('Response:')
                pprint(response)
                connection.send(response)

            elif message.get_service() == 'subtract':
                response = srl.InterfaceMessage('PyService', 'subtract', message.get_client_id())
                response['result'] = int(message['a']) - int(message['b'])
                print('Response:')
                pprint(response)
                connection.send(response)

            # This shouldn't happen.
            else:
                print('ERROR: Invalid service?')

        # This shouldn't happen.
        else:
            print('ERROR: Wrong module?')

    else: time.sleep(1)
