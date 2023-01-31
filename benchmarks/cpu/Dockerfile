# @author Fabio Maschi (Department of Computer Science, ETH Zurich)
# @copyright This software is copyrighted under the BSD 3-Clause License.

FROM nginx

RUN rm /etc/nginx/nginx.conf /etc/nginx/conf.d/default.conf

#COPY /HOST_STATIC_FOLDER /usr/share/nginx/html
COPY nginx.conf /etc/nginx/nginx.conf
COPY mime.types /etc/nginx/conf/mime.types

EXPOSE 80

ENTRYPOINT ["nginx", "-g", "daemon off;"] 