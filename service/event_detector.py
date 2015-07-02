#!/usr/bin/python

import ConfigParser
import datetime as dt
import numpy as np
from fisher import pvalue
import os
import elasticsearch as es
from es_query_generator import es_query_generator
from itertools import izip

class DictParser(ConfigParser.ConfigParser):
    def as_dict(self):
        d = dict(self._sections)
        for k in d:
            d[k] = dict(self._defaults, **d[k])
            d[k].pop('__name__', None)
        return d

class EventDetector:
    cfg = None

    @staticmethod
    def get_counts( start, end, baseline_filters, target_filters, keylist = None, index = None ):
        if index == None: index = EventDetector.cfg['ElasticSearch']['default_index']
        try:
            esi = es.Elasticsearch('https://{}:{}@{}:{}'.format(
                EventDetector.cfg['ElasticSearch']['username'],
                EventDetector.cfg['ElasticSearch']['password'],
                EventDetector.cfg['ElasticSearch']['host'],
                EventDetector.cfg['ElasticSearch']['port']))
            if not esi.ping():
                raise Exception('ERROR: Elasticsearch server did not respond to ping.')

            n           = (end - start).days + 1
            ts_baseline = np.empty(n)
            ts_target   = np.empty(n)
            i           = 0
            for (qb, qt) in izip(
                    es_query_generator(start, end, baseline_filters, keylist),
                    es_query_generator(start, end, target_filters  , keylist)):
                rb = esi.count(index = index, body = qb)
                rt = esi.count(index = index, body = qt)
                ts_baseline[i] = int(rb['count'])
                ts_target[i]   = int(rt['count'])
                i += 1

            return {'baseline': ts_baseline, 'target': ts_target}

        except Exception as e:
            print('ERROR: Could not query elastic search.')
            print(str(e))
            raise

    @staticmethod
    def load_configuration( filename ):
        if os.path.isfile(filename):
            config = DictParser()
            config.read(filename)
            EventDetector.cfg = config.as_dict()
        else:
            print('ERROR: Could not find configuration file "{}"!'.format(filename))
            EventDetector.cfg = []

    @staticmethod
    def temporal_scan( \
            baseline_filters, target_filters, analysis_start, analysis_end,
            keylist = None, cur_window = 7, ref_window = 91, lag = 0,
            index = None):
        start = None
        end   = None

        if EventDetector.cfg == None:
            print('First run - loading configuration "config/tad.cfg"...')
            EventDetector.load_configuration('config/tad.cfg')

        if start is None:
            start = analysis_start - dt.timedelta(days = cur_window + lag + ref_window - 1)

        if end is None:
            end = analysis_end

        print('Querying for counts...')
        counts = EventDetector.get_counts(start, end, baseline_filters, target_filters, keylist, index)
        if isinstance(counts, str):
            raise Exception(counts)
        elif len(counts) == 0:
            raise Exception('ERROR: No results returned. Valid analysis range specified?')

        print('Computing reference and baseline window counts...')
        kernel_ref      = np.ones(ref_window)
        kernel_cur      = np.ones(cur_window)

        n_days = (analysis_end - analysis_start).days + 1

        baseline_ref    = np.correlate(counts['baseline'], kernel_ref)[:n_days]
        target_ref      = np.correlate(counts['target']  , kernel_ref)[:n_days]
        baseline_cur    = np.correlate(counts['baseline'], kernel_cur)[-n_days:]
        target_cur      = np.correlate(counts['target']  , kernel_cur)[-n_days:]

        print('Computing scores...')
        on_date = analysis_start
        results = []
        for si in xrange(n_days):
            p = pvalue(baseline_ref[si], target_ref[si], baseline_cur[si], target_cur[si])

            results.append([
                on_date, baseline_ref[si], target_ref[si], baseline_cur[si],
                target_cur[si], p.left_tail, p.two_tail, p.right_tail])
            on_date += dt.timedelta(days = 1)

        return results

if __name__ == '__main__':
    result = EventDetector.temporal_scan(
        baseline_filters = {'state': 'Colorado'},
        target_filters   = {'state': 'Colorado', 'city': 'Colorado Springs'},
        analysis_start   = dt.datetime.strptime('2014/01/05', '%Y/%m/%d').date(),
        analysis_end     = dt.datetime.strptime('2014/02/05', '%Y/%m/%d').date(),
        cur_window       = 7,
        ref_window       = 91,
        lag              = 0)

    for r in result:
        print(r)
