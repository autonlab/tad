from event_detector import EventDetector
from datetime import datetime

location = 'NORTH_JERSEY_NEW_JERSEY'
keywords = []
start_d  = 'Jan/05/2014'
end_d    = 'Mar/02/2014'

start_dt = datetime.strptime(start_d, '%b/%d/%Y').date()
end_dt   = datetime.strptime(end_d, '%b/%d/%Y').date()

res = EventDetector.cheap_event_report(location, keywords, start_dt, end_dt)
print(res)
