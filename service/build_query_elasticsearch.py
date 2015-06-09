#!/usr/bin/python

from datetime import datetime

def build_query_elasticsearch( start_date, end_date, locations = None, keylist = [], cluster_field = None ):
    # Build query for keylist, if any.
    kl_query = None
    if keylist == None: keylist = ''
    if isinstance(keylist, (list, tuple)): keylist = ' '.join(keylist)
    if len(keylist) > 0:
        kl_query = \
        {
            'query_string':
            {
                'query': keylist,
                'default_operator': 'OR'
            }
        }

    # Build query for date range.
    range_query = \
    {
        'posttime': {
            'gte': start_date.strftime('%Y-%m-%d'),
            'lte': end_date.strftime('%Y-%m-%d')
        }
    }

    # Build queries for the locations.
    l_queries = []
    for location in [x.strip() for x in locations.split(',')]:
        fields = [x.strip() for x in location.split(';')]
        if len(fields) is not 3:
            print('Invalid date: {}.'.format(location))
            return []

        city, state, country = fields
        li_queries = []
        if len(city) > 0: li_queries.append({'term': {'city': city}})
        if len(state) > 0: li_queries.append({'term': {'state': state}})
        if len(country) > 0: li_queries.append({'term': {'country': country}})

        if len(li_queries) == 1:
            l_queries.append(li_queries[0])
        elif len(li_queries) > 1:
            l_queries.append({'and': li_queries})

    # Build filter.
    filter_query = { 'and': [ {'or': l_queries}, {'range': range_query} ] }

    # Now put it all together.
    fields = ['city', 'state', 'country', 'text', 'posttime']
    if isinstance(cluster_field, str) and (len(cluster_field) > 0):
        fields.append(cluster_field)
    final_query = \
    {
        '_source': fields,
        'query': { 'filtered': { 'query': kl_query, 'filter': filter_query } }
    }

    return final_query

if __name__ == '__main__':
    import json

    q = build_query_elasticsearch( \
            datetime.strptime('01/05/2014', '%m/%d/%Y'),
            datetime.strptime('02/05/2014', '%m/%d/%Y'),
            locations = 'Colorado Springs;Colorado;United States,;Colorado;United States',
            keylist = [])
    q['size'] = 5

    print(json.dumps(q, indent=4, separators=(',', ': ')))
