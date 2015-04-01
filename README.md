Temporal Anomaly Detector (TAD)
===============================

Version 1.0.0  
Kyle Miller - mille856@cmu.edu  
Anthony Wertz - awertz@cmu.edu  
Carnegie Mellon University - Auton Lab

Description
-----------

This is a service developed for the DARPA MEMEX program that performs temporal
anomaly detection on advertisments.

Building
--------

The service has two main components that can be built. First, there is the service
itself which can run stand-alone. Second, a docker image can be built (and there
is a read-made image on Dockerhub) to contain the service and the SRL server
application (required for use).

To build the Python `pip` package requred for the service, move to the `service`
directory and run:

```
python setup.py sdist
```

To build the docker container, move to the `samples/docker` directory and run
the `./build-docker` script. Alternatively, you can pull the already built image
from dockerhub with `docker pull autonlab/tad`.

Starting the Python service directly
------------------------------------

The python service will attempt to connect to the SRL server on the local host
(127.0.0.1) on port 12345. There are no script inputs, so if you want to change
these settings you'll need to modify the `tad.py` script.

The script will need the `srl` and `srl_event_detector` packages available which
can be installed with `pip install` globally or (preferably) in a virtual environment.
In addition, `numpy`, `fisher`, and `pyhs2` are also required, all of which can
also be installed via `pip`.

Running the service is achieved simply by

```
python tad.py
```

Note: For using flat files the data is assumed to exists in `snapshot/records.csv`
relative to the working directory. For Hive queries, it is assumed the server
is running on `localhost:10000` and the table `memex_ht_ads_clustered` is
available and properly formatted.

Starting the service using the Docker image
-------------------------------------------

By either building the docker image or by pulling it from dockerhub, you can
load everything pre-configured by simply running the container. The easiest
way to do this is using the script `samples/docker/run-docker`. In general
it should be provided with a log output directory and a flatfile data directory
(even if you're not using flatfiles; the directories must be specified together)
like this:

```
./run-docker my/log/dir my/data/dir
```

After running this, the TCP communication port will not be 12345 on the host
machine (that is the local port in the docker image but not the port on the
docker host). To find the docker port run something like

```
docker inspect autonlab-tad
```

or more specifically

```
docker inspect autonlab-tad | grep -n HostPort | awk '{print $3}' | sed 's/\"//g'
```

to grab the port number to use.

Using the service: Python client example
----------------------------------------

In `samples/client` there is a simple Python client implemented which, with
little or no error checking, will blaze through the process of connecting to
the SRL server, querying the TAD service, displaying the results, and
shutting down the server. This can be used as a general example of how a
query is performed (note, however, you shouldn't shutdown the service after
a query, unless you really don't want it running and providing query support
anymore).

Using the service: Python package
---------------------------------

In conjunction with the SRL Python package `srl`, this library contains a
convinience package `srl_event_detector` used for querying anomaly event
reports from the service. The general approach to a query is

1. Send a request. A status message will be returned indicating that the
   request has been queued. Once a worker process is available it will
   handle the request.
2. Periodically ping for a status update using the task ID provided in
   the reponse from step one. Either a status update will be returned or
   the results if finished.
3. If the results of that task are required again at some point in time,
   just query for the progress of the same task ID.

Anomaly detection event reports are queried using the `CheapEventReportRequestFactory`
class. Progress updates and results are queried using the `TaskProgressMessageFactory`
class. Accessing fields in the class is done using normal array access
operations, e.g. `message['keylist']` for the keyword list.

Using the service: Other languages and Message structure
--------------------------------------------------------

For querying the service in other programming languages, the procedure
is pretty simple. A TCP connection with the server needs to be established
and then you can just start sending messages. All messages are in simple
JSON format with message contents wrapped in a protocol header that looks
like this:

```json
{
   "protocol-version"   : 1000,
   "module"             : "<module name>",
   "service"            : "<service name>",
   "client-id"          : -1,
   "body"               : { }
}
```

This includes the protocol version being used, the module hosting the
service to be performed, the service requested, and the body of the message
(for clients `client-id` can be ignored).

The following are the relevant messages for the TAD service. For all messages,
the module name must be `CMU-TemporalAnomalyDetector`.

### CheapEventReport message

This message must be sent in order to receive an event report. An event
report query is performed to compare ad volume over a specific period
of time with past ad volume, comparing two different regions. In other
words, the report will indicate whether ad volume of the target location
relative to the baseline location in the past is significantly different
than ad volume of the target location to the baseline location in the
period of interest. This is done using the Fisher exact test for
independence using the ad volumes in the reference and current time
periods for the target and baseline locations. The contingency table
looks like this:

            | reference | current
------------|:---------:|:------:
target      | #         | #
baseline    | #         | #

The service field of the header must indicate `CheapEventReport`. The body
contains the following fields.

Field               | Values            | Explanation
--------------------|-------------------|------------
target-location     | string            | Target location(s)
baseline-location   | string            | Baseline location(s)
keylist             | string array      | Search keywords
analysis-start-date | date string       | Start date of analysis period in format %b/%d/%Y
analysis-end-date   | date string       | End date of analysis period
current-window      | int > 0           | Size (in days) of current window
reference-window    | int > 0           | Size (in days) of reference window
lag                 | int >= 0          | Lag (in days) of reference window  behind current window
tailed              | upper, lower, two | Tail for Fisher exact test
data-source         | hive, flatfile    | Source of data to use

The target and baseline locations indicate the locations, separated by
commas, to be used for the target and baseline comparisons. Keylist
(which can be empty) is a set of keywords you wish to search for.
Analysis start and end date represent the period of time your interested
in analysing and are represented using the `strptime` format of `%b/%d/%Y`,
so for example "Mar/14/2015". 

The window sizes represent the windows to aggregate counts in for the
current time period and the reference period. For example, you may compare
the seven days leading up to a date of interest (current-window = 7) to the
previous three months (reference-window = 90). Use the lag parameter if you
want to create more separation between the windows. For example, maybe you
want to compare it to the same time last year. You could set the current
and reference windows to be the same size and the lag to be `365 - current-window`
days long.a

Tailed determines what kind of p-value test to perform with the test. Given
the table configuration, a lower tail test will yield significant p-values
when there appears to be an increase in ad volume in the current time period
of interest in the target location relative to the baseline. An upper tailed
test will yield significant p-values when the ad volume in the current time
period in the target location seems to decrease. Two-tailed test will detect
both of these changes.

The data source allows selection of the hive server or a flatfile to be used.

### Progress message

This message will allow querying for progress on the task. The service field
must indicate `Progress`. The body contains just one important field:

Field   | Values | Explanation
--------|--------|------------
task-id | int    | ID number of the task

The task ID is returned in the response when an event report is requested.
The reponse to this message will include the status of the current task or,
if finished processing, the results of the query.

Handling query results
----------------------

When a query is run, a record is returned for each day within the analysis
time period. Each record is a multi-type array containing the following fields
(in order, accessed by index).

First is the date.

The second is a block of counts for the current time window in the target
and baseline when not considering keywords specified. In this block, the
first column and every second column after that represent the counts in
the target window. The second column and every second column after that
represent the counts in the reference window. There are four such pairs
representing the four ad stratifications: overall, local, new to town, and
first post. Therefore, this block has eight fields ordered as follows:

```
target (overall), reference (overall), target (local), reference (local), ...
```

And, again, these represent counts in the current time window, not taking
into account keywords.

The next block is exactly the same as the last, except it shows counts in
the current time window for ads including the specified keywords. (If you
don't specify keywords these will all be zeros.) The same stratification
as above exists.

There are two more blocks structured identically, representing the counts
in the reference period without and with keywords.

Finally, there are two blocks of four p-values each. The first four are
for the counts without keywords, the last four with. In each block they
are ordered by the same stratification as above: overall, local, new to
town, and first. So that looks like this:

```
p-value (no keywords; overall), p-value (no keywords; local), ...
```

License
-------

The MIT License (MIT)

Copyright (c) 2015 Carnegie Mellon University Auton Lab

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
