#!/usr/bin/python

import ConfigParser
import datetime
import operator
from fisher import pvalue
import elasticsearch as es
from location_matches import location_matches
from build_query_elasticsearch import build_query_elasticsearch

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
    def get_data_flatfile(
            start_date, end_date, locations = None,
            keylist = [], stratify = False, cluster_field = None ):
        intext_keylist = ['_'+x+'_' for x in keylist]
        data = []
        try:
            with open('snapshot/records.csv', 'r') as f:
                if len(keylist) > 0: print(' - Will search for keywords...')
                for row in f:
                    # From
                    #  0      1       2      3    4    5      6       7        8
                    # id  location  state  date  age  size  phone  cluster  content
                    #
                    # To
                    #    0     1      2
                    # cluster date location
                    #
                    # Fake to:
                    #    0     1   2  3  4 5
                    # cluster date 1 loc 1 1

                    fields = row.split(',')

                    # Check location.
                    if (locations != None) and (fields[1] not in locations):
                        continue

                    # Check date.
                    row_date = datetime.datetime.strptime(fields[3], '%b/%d/%Y').date()
                    if (row_date < start_date) or (row_date > end_date):
                        continue

                    # Check that the keyword exists.
                    if len(keylist) > 0:
                        keyword = False
                        for j, x in enumerate(intext_keylist):
                            content = fields[8]
                            if x in content:
                                keyword = True
                                break
                            if content[:len(x)-1] == x[1:]:
                                keyword = True
                                break
                            if content[-len(x)+1:] == x[:-1]:
                                keyword = True
                                break
                    else: keyword = True
                    if not keyword: continue

                    # Add row.
                    dt = datetime.datetime.strptime(fields[3], "%b/%d/%Y").date()
                    cl = fields[7] if stratify else 1
                    data.append([cl, dt, 1, fields[1], 1, 1])

        except Exception as e:
            print('ERROR: Could not query flatfile.')
            print(str(e))
            return []

        return data

    @staticmethod
    def get_data_elasticsearch(
            start_date, end_date, locations = None,
            keylist = [], stratify = False, cluster_field = None ):
        data = []
        try:
            query = build_query_elasticsearch(start_date, end_date, locations, keylist, cluster_field)
            esi = es.Elasticsearch('https://{}:{}@{}:{}'.format(
                EventDetector.cfg['ElasticSearch']['username'],
                EventDetector.cfg['ElasticSearch']['password'],
                EventDetector.cfg['ElasticSearch']['host'],
                EventDetector.cfg['ElasticSearch']['port']))
            if not esi.ping():
                raise Exception('ERROR: Elasticsearch server did not respond to ping.')
            results = esi.search(body=query, size=0)
            print('Results to fetch: {}'.format(results['hits']['total']))
            results = esi.search(body=query, size=results['hits']['total'])
            unknown_id = 1
            for result in results['hits']['hits']:
                dt = datetime.datetime.strptime(result['_source']['posttime'], '%Y-%m-%dT%H:%M:%S').date()
                if stratify:
                    if cluster_field in result['_source']: cl = result['_source']
                    else:
                        cl = 'u{}'.format(unknown_id)
                        unknown_id += 1
                else: cl = 1
                loc = '{};{};{}'.format(
                        result['_source']['city'],
                        result['_source']['state'],
                        result['_source']['country'])
                data.append([cl, dt, 1, loc, 1, 1])

        except Exception as e:
            print('ERROR: Could not query elasticsearch.')
            print(str(e))
            return []

        return data

    @staticmethod
    def get_data(
            start_date, end_date, locations = None,
            keylist = [], stratify = False, cluster_field = None,
            data_source = 'flatfile' ):

        get_data_f = None
        if data_source == 'elasticsearch':
            get_data_f = EventDetector.get_data_elasticsearch
        elif data_source == 'flatfile':
            get_data_f = EventDetector.get_data_flatfile
        else:
            raise Exception('Invalid data source: %s' % data_source)

        return get_data_f(start_date, end_date, locations, keylist, stratify, cluster_field)

    @staticmethod
    def load_configuration( filename ):
        config = DictParser()
        config.read(filename)
        EventDetector.cfg = config.as_dict()

    @staticmethod
    def cheap_event_report( \
            target_location, keylist, analysis_date_start, analysis_date_end,
            baseline_location = '',
            startdate = None, enddate = None, cur_window = 7, ref_window = 91,
            lag = 0, tailed = 'upper', US = False, Canada = False, stratify = False,
            cluster_field = None, data_source = 'flatfile'):

        if EventDetector.cfg == None:
            print('First run - loading configuration')
            EventDetector.load_configuration('config/tad.cfg')

        if stratify: print(' - Will stratify')
        if startdate is None:
            startdate = analysis_date_start - datetime.timedelta(days = cur_window + lag + ref_window - 1)

        if enddate is None:
            enddate = analysis_date_end

        query_locations = None
        if baseline_location != '':
            query_locations = '{0},{1}'.format(baseline_location, target_location)

        print('Querying for data...')
        data = EventDetector.get_data(
                startdate, enddate, query_locations, keylist,
                stratify, cluster_field, data_source)
        if data == []:
            print('WARNING: No query results returned.')
            return []
        print('Found %d data elements.' % len(data))

        adlist = data

        # Columns:
        #    0      1     2
        # location date cluster

        # Columns:
        #    0     1     2      3    4    5
        # cluster date keyword loc state age

        print('Performing stratification...')

        #sort
        adlist.sort(key = operator.itemgetter(0,1))
        first_window = datetime.timedelta(days=1)
        new_window0 = datetime.timedelta(days=1)
        new_window1 = datetime.timedelta(days=7)
        CurClus = -1
        loc_hist = {}
        firstdate = None
        max_loc_hist = None
        local_count = 0
        new_count = 0
        first_count = 0
        newlist=[]
        newlistappend=newlist.append
        for row in adlist:
            date = row[1]
            if row[0]!=CurClus:
                loc_hist = {}
                firstdate = date
                CurClus = row[0]
                loc=row[3]
                loc_hist[loc] = [datetime.date(1900,1,1),date]
                max_loc_hist = (loc_hist[loc][0], loc)
                ntt=2
                first_count+=1

                newlistappend([ntt]+row[1:])
                continue

            loc=row[3]
            if loc in loc_hist:
                if date - loc_hist[loc][1]>=new_window0:
                    loc_hist[loc][0] = loc_hist[loc][1]
                    loc_hist[loc][1] = date
            else:
                loc_hist[loc] = [datetime.date(1900,1,1),date]
            if loc_hist[loc][0]>max_loc_hist[0]:
                max_loc_hist = (loc_hist[loc][0], loc)
            if date - firstdate < first_window:
                ntt=2
                first_count+=1
            elif (date-loc_hist[loc][0]<=new_window1) or (max_loc_hist[1]==loc):
                ntt=0
                local_count+=1
            else:
                ntt=1
                new_count+=1

            newlistappend([ntt] + row[1:])

        print('Generating counts and statistics...')

        count_size = 8 if stratify else 2

        #new-to-town is in newlist, which is list of lists. Fields in everey list in newlist are
        #NewToTown,date,keyword,location,state,age
        countlist = []
        newlist.sort(key = operator.itemgetter(1)) # sort by date
        cur_date = datetime.date(1900,1,1)
        counts = [0]*count_size
        baseline_seq = baseline_location.split(';')
        target_seq   = target_location.split(';')
        for ad in newlist:
            if ad[1] > cur_date:
                countlist.append([cur_date] + counts)
                cur_date=ad[1]
                counts = [0]*count_size

            if location_matches(ad[3], target_seq):
                counts[0] += 1 #target overall
                if stratify:
                    if ad[0] == 0: # local
                        counts[2] += 1
                    elif ad[0] == 1: # new-to-town
                        counts[4] += 1
                    elif ad[0] == 2: # first-appearance
                        counts[6] += 1

            if (baseline_location == '') or location_matches(ad[3], baseline_seq):
                counts[1] += 1 #baseline overall
                if stratify:
                    if ad[0]==0: # local
                        counts[3] += 1
                    elif ad[0] == 1: # new-to-town
                        counts[5] += 1
                    elif ad[0] == 2: # first-appearance
                        counts[7] += 1

        countlist.append([cur_date] + counts)
        del newlist
        idx = len(countlist)-1
        ref_period = \
        [
            countlist[idx][0] - datetime.timedelta(days = cur_window + lag + ref_window-1),
            countlist[idx][0] - datetime.timedelta(days = cur_window + lag),
            len(countlist),
            0
        ]
        cur_period = \
        [
            countlist[idx][0] - datetime.timedelta(days = cur_window - 1),
            countlist[idx][0],
            len(countlist),
            0
        ]
        ref_counts = [0]*count_size
        cur_counts = [0]*count_size
        while 1:
            if countlist[idx][0] >= cur_period[0] and countlist[idx][0] <= cur_period[1]:
                cur_counts = map(operator.add, cur_counts, countlist[idx][1:])
                cur_period[2] = min(cur_period[2], idx)
                cur_period[3] = max(cur_period[3], idx)

            if countlist[idx][0] >= ref_period[0] and countlist[idx][0] <= ref_period[1]:
                ref_counts = map(operator.add, ref_counts, countlist[idx][1:])
                ref_period[2] = min(ref_period[2], idx)
                ref_period[3] = max(ref_period[3], idx)

            idx -= 1
            if countlist[idx][0] < cur_period[0] and countlist[idx][0] < ref_period[0]:
                break

        cur_date = countlist[len(countlist) - 1][0]
        ScanResults = []
        while 1:
            #do test
            if cur_period[1]<=analysis_date_end:
                tests = []
                tests.append(pvalue(
                    ref_counts[0], ref_counts[1], cur_counts[0], cur_counts[1])) # overall
                if stratify:
                    tests.append(pvalue(
                        ref_counts[2], ref_counts[3], cur_counts[2], cur_counts[3])) # local
                    tests.append(pvalue(
                        ref_counts[4], ref_counts[5], cur_counts[4], cur_counts[5])) # new2twn
                    tests.append(pvalue(
                        ref_counts[6], ref_counts[7], cur_counts[6], cur_counts[7])) # first

                if tailed == 'upper':
                    res = [cur_period[1]] + cur_counts + ref_counts + [t.right_tail for t in tests]
                elif tailed == 'lower':
                    res=[cur_period[1]] + cur_counts + ref_counts + [t.left_tail for t in tests]
                else:
                    res = [cur_period[1]] + cur_counts + ref_counts + [t.two_tail for t in tests]

                ScanResults.append(res)

            cur_period[0]-=datetime.timedelta(days=1)
            cur_period[1]-=datetime.timedelta(days=1)
            ref_period[0]-=datetime.timedelta(days=1)
            ref_period[1]-=datetime.timedelta(days=1)

            if cur_period[1] < analysis_date_start: break

            #update
            if cur_period[2] > 0 and countlist[cur_period[2]-1][0] >= cur_period[0]:
                cur_counts = map(operator.add, cur_counts, countlist[cur_period[2]-1][1:])
                cur_period[2] -= 1

            if ref_period[2] > 0 and countlist[ref_period[2]-1][0] >= ref_period[0]:
                ref_counts = map(operator.add, ref_counts, countlist[ref_period[2]-1][1:])
                ref_period[2] -= 1

            #downdate
            if cur_period[3] > 0 and countlist[cur_period[3]][0] > cur_period[1]:
                cur_counts = map(operator.sub, cur_counts, countlist[cur_period[3]][1:])
                cur_period[3] -= 1

            if ref_period[3] > 0 and countlist[ref_period[3]][0] > ref_period[1]:
                ref_counts = map(operator.sub, ref_counts, countlist[ref_period[3]][1:])
                ref_period[3] -= 1

        return ScanResults

if __name__ == '__main__':
    result = EventDetector.cheap_event_report(
        'Colorado Springs;Colorado;United States', None,
        datetime.datetime.strptime('2014/01/05', '%Y/%m/%d').date(),
        datetime.datetime.strptime('2014/02/05', '%Y/%m/%d').date(),
        baseline_location = ';Colorado;United States',
        cur_window      = 7,
        ref_window      = 91,
        lag             = 0,
        tailed          = 'upper',
        stratify        = True,
        cluster_field   = 'phone',
        data_source     = 'elasticsearch')

    for r in result:
        print(r)
