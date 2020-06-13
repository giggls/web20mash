FROM debian:buster

ENV PACKAGE web20mash_4.2.1_amd64.deb

# Default to UTF-8 file.encoding
ENV LANG C.UTF-8

COPY $PACKAGE /tmp/

RUN apt-get update; apt-get install -y /tmp/${PACKAGE} && rm /tmp/${PACKAGE}

COPY mashctld.conf.simulation /etc/mashctld.conf

RUN sed -e 's/port *=.*/port = 54321/g' -i /etc/mashctld.conf

CMD ["/usr/bin/mashctld", "-u", "nobody"]
