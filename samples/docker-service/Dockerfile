########## DON'T MODIFY THIS SECTION, SCROLL DOWN ###########
# The next configurations do some configuring that can take a
# bit of time, so best make modifications AFTER them to allow
# utilization of cached steps.
#
# Note: As of 2015/03/05 this is Ubuntu 14.04.
FROM dockerfile/java:oracle-java7
MAINTAINER Kyle Miller <mille856@andrew.cmu.edu>

#------------ Hadoop and Hive configuration ---------------#
# Grab the right Hadoop and Hive versions, then install.
RUN wget -q https://archive.apache.org/dist/hadoop/core/hadoop-2.5.0/hadoop-2.5.0.tar.gz
RUN wget -q http://mirror.reverse.net/pub/apache/hive/hive-0.13.1/apache-hive-0.13.1-bin.tar.gz

RUN tar -xf hadoop-2.5.0.tar.gz --dir /usr/lib
RUN ln -s /usr/lib/hadoop-2.5.0 /usr/lib/hadoop
RUN chown -R root:root /usr/lib/hadoop

RUN tar -xf apache-hive-0.13.1-bin.tar.gz --dir /usr/lib
RUN ln -s /usr/lib/apache-hive-0.13.1-bin /usr/lib/hive
RUN chown -R root:root /usr/lib/hive

ENV HADOOP_HOME /usr/lib/hadoop
ENV HIVE_HOME /usr/lib/hive
ENV PATH $PATH:$HADOOP_HOME/bin:$HIVE_HOME/bin

#------------ Install development software --------------#
RUN apt-get update && apt-get install -y \
        python python-dev python-pip python-numpy
RUN pip install fisher

#------------- MEMEX Configuration --------------#
COPY guest-cfg/. /tmp/config/
RUN cd /tmp/config && ./copy-files.sh memex

########### MODIFY BELOW THIS ##############

#------------- Service Configuration --------------#
# Expose the service port.
EXPOSE 12345

# Copy service applications.
COPY service /service

# Install the python libraries.
RUN pip install /service/SRL/dist/*.tar.gz
RUN pip install /service/eventdetector/package/dist/*.tar.gz

# Create the service entry point.
ENTRYPOINT ["/service/run"]
