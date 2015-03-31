import datetime
import calendar
import operator
from fisher import pvalue
import pyhs2
import re

class EventDetector:
    @staticmethod
    def get_data( start_date, end_date, locations = None ):
        # If locations provided, verify contents and build query string.
        location_query = ''
        if locations != None:
            if re.search('[^\w_,]', locations) != None: return []
            locations = ','.join("'{0}'".format(l.strip()) for l in locations.split(',') if l.strip() != '')
            if locations != '':
                location_query = 'and location in ({0})'.format(locations)

        # Find data.
        data = []
        try:
            with pyhs2.connect( \
                    host='localhost', port=10000, authMechanism='PLAIN',
                    user='hive', password='hive', database='default') as con:
                with con.cursor() as cur:
                    start_ts    = calendar.timegm(
                            datetime.datetime.combine(start_date, datetime.datetime.min.time()).timetuple())
                    end_ts      = calendar.timegm(
                            datetime.datetime.combine(end_date, datetime.datetime.min.time()).timetuple())
                    fields      = 'location,state,date,age,size,cluster,keywords'
                    table       = 'memex_ht_ads_clustered'
                    query       = 'select %s from %s where timestamp >= %d and timestamp <= %d %s' \
                            % (fields, table, start_ts, end_ts, location_query)
                    print('Querying hive: %s' % query)
                    cur.execute(query)
                    data = cur.fetchall()
        except Exception as e:
            print('ERROR: Could not query hive.')
            print(str(e))
            return []

        return data

    @staticmethod
    def cheap_event_report( \
            target_location, keylist, analysis_date_start, analysis_date_end,
            baseline_location = '',
            startdate = None, enddate = None, cur_window = 7, ref_window = 91,
            lag = 0, tailed = 'upper', US = False, Canada = False):
        if startdate is None:
            startdate = analysis_date_start - datetime.timedelta(days = cur_window + lag + ref_window - 1)

        if enddate is None:
            enddate = analysis_date_end

        intext_keylist      = ['_'+x+'_' for x in keylist]

        query_locations = None
        if baseline_location != '':
            query_locations = '{0},{1}'.format(baseline_location, target_location)

        print('Querying for data...')
        data = EventDetector.get_data(startdate, enddate, query_locations)
        if data == []:
            print('WARNING: No query results returned.')
            return []
        print('Found %d data elements.' % len(data))

        print('Filtering data...')
        adlist = []
        adlistadd = adlist.append

        # Columns:
        #  0     1     2     3    4       5        6
        # loc, state, date, age, size, cluster, content

        for fields in data:
            #look for keywords
            keyword = 0
            content = fields[6]
            for j,x in enumerate(intext_keylist):
                #check in text
                if x in content:
                    keyword = 1
                    break

                #check at start of line
                if content[:len(x)-1] == x[1:]:
                    keyword = 1
                    break

                #check at end of line
                if content[-len(x)+1:] == x[:-1]:
                    keyword = 1
                    break

            # Convert date.
            dt = datetime.datetime.strptime(fields[2], "%b/%d/%Y").date()

            mycluster = fields[5]
            adlistadd([mycluster, dt, keyword] + fields[0:2] + fields[3:4])

        print('Sorting...')

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

        print('Working...')

        #new-to-town is in newlist, which is list of lists. Fields in everey list in newlist are
        #NewToTown,date,keyword,location,state,age
        countlist = []
        newlist.sort(key = operator.itemgetter(1)) # sort by date
        cur_date = datetime.date(1900,1,1)
        counts = [0]*16
        for ad in newlist:
            if ad[1] > cur_date:
                countlist.append([cur_date] + counts)
                cur_date=ad[1]
                counts = [0]*16

            if ad[3] in target_location:
                counts[0] += 1 #target overall
                if ad[2]: counts[8] += 1
                if ad[0] == 0: # local
                    counts[2] += 1
                    if ad[2]: counts[10] += 1
                elif ad[0] == 1: # new-to-town
                    counts[4] += 1
                    if ad[2]: counts[12] += 1
                elif ad[0] == 2: # first-appearance
                    counts[6] += 1
                    if ad[2]: counts[14] += 1

            if (baseline_location == '') or (ad[3] in baseline_location):
                counts[1] += 1 #baseline overall
                if ad[2]: counts[9] += 1
                if ad[0]==0: # local
                    counts[3] += 1
                    if ad[2]: counts[11] += 1
                elif ad[0] == 1: # new-to-town
                    counts[5] += 1
                    if ad[2]: counts[13]+=1
                elif ad[0] == 2: # first-appearance
                    counts[7] += 1
                    if ad[2]: counts[15] += 1

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
        ref_counts = [0]*16
        cur_counts = [0]*16
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
                tests.append(pvalue(
                    ref_counts[2], ref_counts[3], cur_counts[2], cur_counts[3])) # local
                tests.append(pvalue(
                    ref_counts[4], ref_counts[5], cur_counts[4], cur_counts[5])) # new2twn
                tests.append(pvalue(
                    ref_counts[6], ref_counts[7], cur_counts[6], cur_counts[7])) # first
                tests.append(pvalue(
                    ref_counts[8], ref_counts[9], cur_counts[8], cur_counts[9])) # overall+keyword
                tests.append(pvalue(
                    ref_counts[10], ref_counts[11], cur_counts[10], cur_counts[11])) #local+keyword
                tests.append(pvalue(
                    ref_counts[12], ref_counts[13], cur_counts[12], cur_counts[13])) #new2twn+keyword
                tests.append(pvalue(
                    ref_counts[14], ref_counts[15], cur_counts[14], cur_counts[15])) #first+keyword

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
