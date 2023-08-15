# NGINX with SDRaD

This repository contains the source code of a use case for SDRaD: NGINX 

## How to get started

This repository can be cloned using the following commands:

```
git clone -b sdrad git@github.com:secure-rewind-and-discard/nginx.git
```
`sdrad` branch is based on nginx-1.23.1-RELEASE. 

The detailed instruction for compiling NGINX from source code can be found [here](http://nginx.org/en/docs/configure.html).

In order to have rollback capability:
- NGINX should be compiled with SDRaD library `--with-ld-opt='-L../src/sdrad -lsdrad`. Please make sure that Linux dynamic linker search path contains the SDRaD library path. Follow the [instructions](https://github.com/secure-rewind-and-discard/secure-rewind-and-discard). Note that NGINX is a multi-processing application; the SDRaD library does not need to be compiled with support `SDRAD_MULTITHREAD` and `ZERO_DOMAIN` options. 
- NGINX should be compiled with two wrapper options for two functions: `-Wl,--wrap=ngx_http_parse_request_line -Wl,--wrap=ngx_http_parse_header_line'`

NGINX Configure suggestions: 
- Use `-Wno-error`: The source code of release 1.23.1 cannot be compiled without it. 

An example configure command is provided below. Run the configure script as follows:

```
./auto/configure --prefix=/path/to/opt --with-cc-opt="-Wimplicit-fallthrough=0 -Wno-error -fstack-protector-strong -I ../secure-rewind-and-discard/src/  -O2"   --without-http_rewrite_module --without-http_gzip_module --with-http_ssl_module  --with-ld-opt='-L../secure-rewind-and-discard/src -lsdrad -Wl,--wrap=ngx_http_parse_request_line -Wl,--wrap=ngx_http_parse_header_line' 
```
Compile NGINX
```
make -j4 
``` 
Run NGINX. The different Nginx configuration files can be found in [benchmark](./benchmark/confs)
``` 
./obj/nginx -c /path/to/nginx.conf.1
``` 

## Benchmark 

Execute [./create_content.sh](./benchmark/datasets/create_content.sh) to build benchmark datasets.


[ab - Apache HTTP server benchmarking tool](https://httpd.apache.org/docs/2.4/programs/ab.html) is used. The following parameter is used to generate the paper results. 

``` 
/path/to/ab -k 100000000 -t 65 -c 75 -n 100000000 -g /path/to/data.abbench.4  $url/$file > ./$output_dir/ab.out.4 2>&1 &

``` 





 








