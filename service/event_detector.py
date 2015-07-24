#!/usr/bin/python

import ConfigParser
import datetime as dt
import numpy as np
from fisher import pvalue
import os
import elasticsearch as es
from es_query_generator import es_query_generator
from itertools import izip
import json

class DictParser(ConfigParser.ConfigParser):
    def as_dict(self):
        d = dict(self._sections)
        for k in d:
            d[k] = dict(self._defaults, **d[k])
            d[k].pop('__name__', None)
        return d

def get_or( obj, field, default ):
    return obj[field] if field in obj else default

class EventDetector:
    cfg = None

    @staticmethod
    def get_counts( start, end, baseline_filters, target_filters,
            keylist = None, index = None, time_field = None, constant_baseline = False ):
        esconfig = EventDetector.cfg['ElasticSearch']
        if index == None: index = get_or(esconfig, 'default_index', '')
        if time_field == None: time_field = get_or(esconfig, 'time_field', 'date')
        try:
            protocol    = get_or(esconfig, 'protocol', 'https')
            usernm      = get_or(esconfig, 'username', '')
            passwd      = get_or(esconfig, 'password', '')
            if len(passwd) > 0: passwd = ':' + passwd + '@'
            elif len(usernm) > 0: usernm = usernm + '@'
            host        = get_or(esconfig, 'host', 'localhost')
            port        = get_or(esconfig, 'port', '9200')
            esi = es.Elasticsearch('{}://{}{}{}:{}'.format(
                protocol, usernm, passwd, host, port))
            if not esi.ping():
                raise Exception('ERROR: Elasticsearch server did not respond to ping.')

            n           = (end - start).days + 1
            ts_baseline = np.empty(n)
            ts_target   = np.empty(n)

            if constant_baseline: ts_baseline.fill(1)
            else:
                qb = es_query_generator(start, end, baseline_filters, keylist, time_field)
                rb = esi.search(index = index, body = qb)
                for idx, val in enumerate(rb['aggregations']['counts']['buckets']):
                    ts_baseline[idx] = int(val['doc_count'])

            qt = es_query_generator(start, end, target_filters  , keylist, time_field)
            rt = esi.search(index = index, body = qt)
            for idx, val in enumerate(rt['aggregations']['counts']['buckets']):
                ts_target[idx] = int(val['doc_count'])

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
            keylist = None, cur_window = 7, ref_window = 91, lag = 0, constant_baseline = False,
            index = None, time_field = None):
        start = None
        end   = None

        if EventDetector.cfg == None:
            EventDetector.load_configuration('config/tad.cfg')

        if start is None:
            start = analysis_start - dt.timedelta(days = cur_window + lag + ref_window - 1)

        if end is None:
            end = analysis_end

        counts = EventDetector.get_counts(
                start, end, baseline_filters, target_filters,
                keylist, index, time_field, constant_baseline)
        if isinstance(counts, str):
            raise Exception(counts)
        elif len(counts) == 0:
            raise Exception('ERROR: No results returned. Valid analysis range specified?')

        kernel_ref      = np.ones(ref_window)
        kernel_cur      = np.ones(cur_window)

        n_days = (analysis_end - analysis_start).days + 1

        baseline_ref    = np.correlate(counts['baseline'], kernel_ref)[:n_days]
        target_ref      = np.correlate(counts['target']  , kernel_ref)[:n_days]
        baseline_cur    = np.correlate(counts['baseline'], kernel_cur)[-n_days:]
        target_cur      = np.correlate(counts['target']  , kernel_cur)[-n_days:]

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
        baseline_filters = {},
        target_filters   = {'Location': 'MINOT_NORTH_DAKOTA'},
        analysis_start   = dt.datetime.strptime('2013-07-25', '%Y-%m-%d').date(),
        analysis_end     = dt.datetime.strptime('2013-07-27', '%Y-%m-%d').date(),
        cur_window       = 1,
        ref_window       = 1,
        lag              = 0,
        index            = 'trafficking',
        time_field       = 'Date')

    for r in result:
        print(r)
