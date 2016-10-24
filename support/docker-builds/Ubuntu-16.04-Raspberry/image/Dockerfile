FROM scratch
ADD arm-root.tar /
ADD entrypoint /usr/bin/
ENV PATH /sbin:/usr/sbin:/bin:/usr/bin:/usr/local/bin:$PATH
ENTRYPOINT ["/usr/bin/entrypoint"]
 
