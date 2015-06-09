from celery import Celery
from flask import Flask
from flask_restful import Resource, Api, reqparse, inputs
from datetime import datetime
from event_detector import EventDetector
import argparse

class ListServices(Resource):
    handle      = 'list'
    description = 'List available services.'
    uris        = [x.format(handle) for x in ['/{}']]
    result      = None

    def get( self ):
        if self.result == None:
            global services
            self.result = []
            for service in services:
                self.result.append({'handle': service.handle, 'description': service.description})

        return {'services': self.result}

class EventReportService(Resource):
    handle      = 'event-report'
    description = 'Create an event report for a given query.'
    uris        = [x.format(handle) for x in ['/{}', '/{}/<string:task_id>']]

    def get( self, task_id = None ):
        global tasks, res_q, task_update_lock

        if task_id == None:
            return {'error': 'Task ID must be set for GET'}, 400

        if task_id not in tasks:
            return {'error': 'Task {} not found'.format(task_id)}, 400

        # If task is not yet complete, update fields.
        if not tasks[task_id]['finished']:
            task = worker.AsyncResult(task_id)
            if task.state == 'PENDING':
                pass
            elif task.state == 'FAILURE':
                tasks[task_id]['finished'] = True
                tasks[task_id]['status'] = 'Failed'
                tasks[task_id]['error'] = task.info
            else:
                if task.info == None:
                    return {'error': str(task)}

                for field in ['status', 'progress', 'error', 'result']:
                    if field in task.info:
                        tasks[task_id][field] = task.info.get(field, tasks[task_id][field])

        return {
            'task-id' : task_id,
            'status'  : tasks[task_id]['status'],
            'progress': tasks[task_id]['progress'],
            'error'   : tasks[task_id]['error'],
            'result'  : tasks[task_id]['result'] }

    def post( self, task_id = None ):
        global tasks

        if task_id != None:
            return {'error': 'Task ID should not be sent in POST'}, 400

        # Parse query request.
        parser = reqparse.RequestParser(bundle_errors = True)
        parser.add_argument('target-location'       , required=True)
        parser.add_argument('baseline-location'     , required=False,
                default='')
        parser.add_argument('keylist'               , required=False,
                type=str, default=[], action='append')
        parser.add_argument('analysis-start-date'   , required=True)
        parser.add_argument('analysis-end-date'     , required=True)
        parser.add_argument('current-window'        , required=False,
                type=int, default=7)
        parser.add_argument('reference-window'      , required=False,
                type=int, default=91)
        parser.add_argument('lag'                   , required=False,
                type=int, default=0)
        parser.add_argument('tailed'                , required=False,
                type=str, default='upper', choices=['lower', 'upper', 'two'])
        parser.add_argument('data-source'           , required=False,
                type=str, default='elasticsearch')
        parser.add_argument('stratify'              , required=False,
                type=inputs.boolean, default=False)
        parser.add_argument('cluster-field'         , required=False,
                type=str, default='phone')
        args = parser.parse_args(strict = True)

        if args['lag'] < 0: return {'error': 'lag cannot be negative'}, 400
        if args['current-window'] < 1: return {'error': 'current-window size must be postive'}, 400
        if args['reference-window'] < 1: return {'error': 'reference-window size must be postive'}, 400

        pargs = {}

        try: pargs['analysis-start-date'] = datetime.strptime(args['analysis-start-date'], '%Y/%m/%d').date()
        except: return {'error': 'analysis-start-date is invalid: {}'.format(args['analysis-start-date'])}, 400

        try: pargs['analysis-end-date'] = datetime.strptime(args['analysis-end-date'], '%Y/%m/%d').date()
        except: return {'error': 'analysis-end-date is invalid: {}'.format(args['analysis-end-date'])}, 400

        if pargs['analysis-start-date'] >= pargs['analysis-end-date']:
            return {'error': 'analysis-end-date must come after analysis-start-date'}, 400

        task = {
            'args'      : args,
            'pargs'     : pargs,
            'progress'  : 0,
            'status'    : 'Not started',
            'error'     : None,
            'finished'  : False,
            'result'    : None }

        ctask = worker.apply_async(args=[task])
        tasks[ctask.id] = task

        return {'task-id': ctask.id}

# Start
app = Flask(__name__)
app.config['CELERY_BROKER_URL'] = 'amqp://guest:guest@localhost:5672//'
app.config['CELERY_RESULT_BACKEND'] = 'amqp://guest:guest@localhost:5672//'

celery = Celery(app.name, broker=app.config['CELERY_BROKER_URL'])
celery.conf.update(app.config)

api = Api(app)

@celery.task(bind=True)
def worker( self, task ):
    self.update_state(state='PROGRESS', meta={'status': 'In progress'})
    try:
        args  = task['args']
        pargs = task['pargs']
        result = EventDetector.cheap_event_report(
                args['target-location'], args['keylist'],
                pargs['analysis-start-date'], pargs['analysis-end-date'],
                args['baseline-location'],
                cur_window      = args['current-window'],
                ref_window      = args['reference-window'],
                lag             = args['lag'],
                tailed          = args['tailed'],
                stratify        = args['stratify'],
                cluster_field   = args['cluster-field'],
                data_source     = args['data-source'])
    except Exception as e:
        return {
            'finished': True,
            'status': 'Failed',
            'error': 'Exception occurred running event report: {}'.format(str(e))
        }

    # Return.
    for i in xrange(len(result)):
        result[i][0] = result[i][0].strftime('%Y/%m/%d')
    return {'progress': 100, 'status': 'Finished', 'finished': True, 'result': result}

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Start temporal anomaly scan service.')
    parser.add_argument('--debug', '-d', action='store_true')
    args = parser.parse_args()
    if args.debug:
        print('Starting in DEBUG mode')

    tasks = {}

    services = [ListServices, EventReportService]
    for service in services:
        api.add_resource(service, *service.uris)

    try:
        app.run(debug = args.debug)
    except KeyboardInterrupt:
        pass

