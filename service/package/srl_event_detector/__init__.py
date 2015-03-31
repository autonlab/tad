import srl
import re
from datetime import datetime

ServiceName = 'PyEventDetector'

class TaskProgressMessageFactory:
    @staticmethod
    def generate( original_message, progress, status_message, task_id = None ):
        if task_id == None: task_id = original_message['task-id']
        return srl.ProgressMessageFactory.generate(
                original_message, task_id, progress, status_message)

    @staticmethod
    def generate_progress_request( task_id ):
        return srl.ProgressMessageFactory.generate_progress_request(
                ServiceName, 'Progress', task_id)

class CheapEventReportRequestFactory:
    @staticmethod
    def generate( \
            target_location, keylist, analysis_start_date, analysis_end_date,
            baseline_location = '',
            cur_window = 7, ref_window = 91, lag = 0, tailed = 'lower'):
        if not isinstance(target_location, str):
            raise Exception('target_location should be a string!')
        if not isinstance(keylist, (list, tuple)):
            keylist = [keylist]
        if not isinstance(analysis_start_date, str):
            raise Exception('analysis_start_date should be a string!')
        if not isinstance(analysis_end_date, str):
            raise Exception('analysis_end_date should be a string!')
        if tailed not in ['lower', 'upper', 'two']:
            raise Exception('tailed should be from the set [lower, upper, two]!')

        message = srl.InterfaceMessage(ServiceName, 'CheapEventReport')
        message['target-location']      = target_location
        message['baseline-location']    = baseline_location
        message['keylist']              = keylist
        message['analysis-start-date']  = analysis_start_date
        message['analysis-end-date']    = analysis_end_date
        message['current-window']       = cur_window
        message['reference-window']     = ref_window
        message['lag']                  = lag
        message['tailed']               = tailed
        return message.encode()

    @staticmethod
    def parse( message ):
        if not isinstance(message, srl.InterfaceMessage):
            raise Exception('message should be an InterfaceMessage!')

        request = \
        {
            'target-location'       : message['target-location'],
            'baseline-location'     : message['baseline-location'],
            'keylist'               : message['keylist'],
            'analysis-start-date'   : datetime.strptime(message['analysis-start-date']  , '%b/%d/%Y').date(),
            'analysis-end-date'     : datetime.strptime(message['analysis-end-date']    , '%b/%d/%Y').date(),
            'current-window'        : int(message['current-window']),
            'reference-window'      : int(message['reference-window']),
            'lag'                   : int(message['lag']),
            'tailed'                : message['tailed'].lower(),
        }

        # Dates should be in order and not the same.
        if request['analysis-start-date'] >= request['analysis-end-date']:
            raise Exception('Invalid analysis date range.')

        # Windows and lag should be positive.
        if request['current-window'] < 1:
            raise Exception('current-window invalid (non-positive integer).')
        if request['reference-window'] < 1:
            raise Exception('reference-window invalid (non-positive integer).')
        if request['lag'] < 0:
            raise Exception('lag invalid (negative integer).')

        # tailed must be valid.
        if request['tailed'] not in ['lower', 'upper', 'two']:
            raise Exception('tailed invalid ("%s" not in set [lower, upper, two]' % request['tailed'])

        # Make sure the locations contain no illegal characters.
        if re.search('[^\w_,]', ','.join([request['target-location'], request['baseline-location']])) != None:
            raise Exception('Invalid character in target- or baseline-location!')

        return request

class CheapEventReportResponseFactory:
    @staticmethod
    def generate( original_message, results ):
        if not isinstance(original_message, srl.InterfaceMessage):
            raise Exception('original_message should be an InterfaceMessage!')

        message = srl.InterfaceMessage(
                original_message.get_module(),
                original_message.get_service(),
                original_message.get_client_id())
        message['results'] = results
        for i in xrange(len(message['results'])):
            if not isinstance(message['results'][i][0], str):
                message['results'][i][0] = message['results'][i][0].strftime('%m/%d/%Y')
        return message.encode()
