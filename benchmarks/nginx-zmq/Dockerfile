# @author Fabio Maschi (Department of Computer Science, ETH Zurich)
# @copyright This software is copyrighted under the BSD 3-Clause License.

#FROM debian:bullseye-slim
FROM xilinx/xilinx_runtime_base:alveo-2022.1-ubuntu-18.04

ENV NGINX_VERSION 1.24.0

RUN apt update -y && \
    apt install -y wget g++ libzmq3-dev make supervisor && \
    wget https://nginx.org/download/nginx-$NGINX_VERSION.tar.gz && \
    wget https://github.com/PCRE2Project/pcre2/releases/download/pcre2-10.42/pcre2-10.42.tar.gz && \
    tar -xzf nginx-$NGINX_VERSION.tar.gz && \
    tar -xzf pcre2-10.42.tar.gz

COPY . .

RUN cd nginx-$NGINX_VERSION && \
    ./configure --prefix=/etc/nginx \
    --add-module=/ngx_http_zmq_module \
    --with-pcre=../pcre2-10.42 \
    --without-http_access_module \
    --without-http_auth_basic_module \
    --without-http_autoindex_module \
    --without-http_browser_module \
    --without-http_charset_module \
    --without-http_empty_gif_module \
    --without-http_fastcgi_module \
    --without-http_geo_module \
    --without-http_gzip_module \
    --without-http_limit_conn_module \
    --without-http_limit_req_module \
    --without-http_map_module \
    --without-http_memcached_module \
    --without-http_proxy_module \
    --without-http_referer_module \
    --without-http_scgi_module \
    --without-http_ssi_module \
    --without-http_split_clients_module \
    --without-http_upstream_hash_module \
    --without-http_upstream_ip_hash_module \
    --without-http_upstream_least_conn_module \
    --without-http_upstream_zone_module \
    --without-http_userid_module \
    --without-http_uwsgi_module \
    --without-http-cache \
    --without-mail_pop3_module \
    --without-mail_imap_module \
    --without-mail_smtp_module && \
    make && make install && cd .. && \
    rm /etc/nginx/conf/fastcgi.conf && \
    rm /etc/nginx/conf/fastcgi.conf.default && \
    rm /etc/nginx/conf/fastcgi_params && \
    rm /etc/nginx/conf/fastcgi_params.default && \
    rm /etc/nginx/conf/koi-utf && \
    rm /etc/nginx/conf/koi-win && \
    rm /etc/nginx/conf/mime.types && \
    rm /etc/nginx/conf/mime.types.default && \
    rm /etc/nginx/conf/nginx.conf && \
    rm /etc/nginx/conf/nginx.conf.default && \
    rm /etc/nginx/conf/scgi_params && \
    rm /etc/nginx/conf/scgi_params.default && \
    rm /etc/nginx/conf/uwsgi_params && \
    rm /etc/nginx/conf/uwsgi_params.default && \
    rm /etc/nginx/conf/win-utf && \
    mv ./nginx.conf /etc/nginx/conf/nginx.conf

ENV PATH="/etc/nginx/sbin:$PATH"

EXPOSE 80
ENV XILINX_XRT="/opt/xilinx/xrt_versions/xrt_202210.2.13.466"

ENTRYPOINT ["/usr/bin/supervisord"]