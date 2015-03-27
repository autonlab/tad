# coding: utf-8
import json
import socket

# This will naïvely attempt to segment a message into blocks representing JSON
# structures. In the event two or more messages are received from the buffer
# at once, this allows processing them separetely (otherwise the JSON parser
# will complain).
def extract_json_blocks( message ):
    # Should be a string.
    if not isinstance(message, str):
        raise Exception('message should be of type str!')

    # Need at least two characters for a JSON block. {}
    if len(message) < 2:
        return []

    # Extract the blocks.
    blocks = []
    opened = 0
    bs     = 0
    for ci in xrange(len(message)):
        if message[ci] == '{':
            opened += 1
            if opened == 1: bs = ci
        elif message[ci] == '}':
            opened -= 1
            if opened == 0:
                blocks.append(message[bs:ci + 1])
            elif opened < 0: opened = 0

    return blocks

class InterfaceMessage:
    def __init__( self, module = '', service = '', client_id = -1 ):
        self.fields = \
        {
            'protocol-version'  : 1000,
            'module'            : module,
            'service'           : service,
            'client-id'         : client_id,
            'body'              : {}
        }

    def decode( self, raw_message ):
        self.fields = json.loads(raw_message)
        return self

    def encode( self ):
        return json.dumps(self.fields)

    def get_protocol_version( self ):
        return self.fields['protocol-version']

    def get_module( self ):
        return self.fields['module']

    def set_module( self, module ):
        self.fields['module'] = module

    def get_service( self ):
        return self.fields['service']

    def set_service( self, service ):
        self.fields['service'] = service

    def get_client_id( self ):
        return self.fields['client-id']

    def set_client_id( self, client_id ):
        self.fields['client-id'] = client_id;

    def __contains__( self, i ):
        return i in self.fields['body']

    def __getitem__( self, i ):
        return self.fields['body'][i]

    def __setitem__( self, i, value ):
        self.fields['body'][i] = value

    def __delitem__( self, i ):
        del self.fields['body'][i]

class ErrorMessageFactory:
    @staticmethod
    def generate( original_message, error_message ):
        if not isinstance(original_message, InterfaceMessage):
            raise Exception('original_message should be an instance of class InterfaceMessage!')

        message = InterfaceMessage(
                original_message.get_module(),
                original_message.get_service(),
                original_message.get_client_id())
        message['error'] = error_message
        message['original-message'] = original_message.fields
        return message.encode()

class StatusMessageFactory:
    @staticmethod
    def generate( original_message, status_message ):
        if not isinstance(original_message, InterfaceMessage):
            raise Exception('original_message should be an instance of class InterfaceMessage!')

        message = InterfaceMessage(
                original_message.get_module(),
                original_message.get_service(),
                original_message.get_client_id())
        message['status'] = status_message
        message['original-message'] = original_message.fields
        return message.encode()

class ProgressMessageFactory:
    @staticmethod
    def generate( original_message, task_id, progress, status_message ):
        if not isinstance(original_message, InterfaceMessage):
            raise Exception('original_message should be an instance of class InterfaceMessage!')

        message = InterfaceMessage(
                original_message.get_module(),
                original_message.get_service(),
                original_message.get_client_id())
        message['task-id']          = task_id
        message['status']           = status_message
        message['progress']         = float(progress)
        message['original-message'] = original_message.fields
        return message.encode()

    @staticmethod
    def generate_progress_request( module, service, task_id ):
        message = InterfaceMessage(module, service)
        message['task-id'] = task_id
        return message.encode()

class ShutdownMessageFactory:
    @staticmethod
    def generate( ):
        message = InterfaceMessage('Builtin', 'Shutdown')
        return message.encode()

class NoOpMessageFactory:
    @staticmethod
    def generate( ):
        message = InterfaceMessage('Builtin', 'NoOp')
        return message.encode()

class RegisterServiceMessageFactory:
    @staticmethod
    def generate( provider_name, services ):
        if not isinstance(services, (list, tuple)):
            services = [services]
        message = InterfaceMessage('Builtin', 'RegisterService')
        message['provider-name'] = provider_name
        message['services'] = services
        return message.encode()

class TCPConnection:
    def __init__( self, host, service = None ):
        if host.count(':') > 1:
            raise Exception('host should be a valid host name or IP with optional port.')
        if host.count(':') == 1: host, port = host.split(':')
        elif service != None: port = socket.getservbyname(service)
        else: raise Exception('A port or a service name must be provided!')

        self.host = socket.gethostbyname(host)
        self.port = int(port)
        self.socket = None

        self.connect()

    def __del__( self ):
        self.disconnect()

    def connect( self ):
        if self.socket != None: self.disconnect()

        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socket.connect((self.host, self.port))
        self.socket.setblocking(0)

    def disconnect( self ):
        if self.socket != None:
            self.socket.close()
            self.socket = None

    def send( self, message ):
        if self.socket == None: raise Exception('No connection established.')
        if isinstance(message, InterfaceMessage):
            message = message.encode()
        if not isinstance(message, str):
            raise Exception('The send routine can accept a string or an InterfaceMessage only.')

        bytes_sent = 0
        while bytes_sent < len(message):
            sent = self.socket.send(message[bytes_sent:])
            if not sent: raise Exception('Connection was lost')
            bytes_sent += sent
        return bytes_sent

    def receive( self ):
        if self.socket == None: raise Exception('No connection established.')

        data = ''
        while True:
            try:
                d = self.socket.recv(8192)
                if not d: break
                data += d
            except:
                break
        return data
