FROM rsmmr/clang
LABEL maintainer "lxiange@gmail.com"

COPY . /rigel

WORKDIR /rigel

RUN apt install -y m4 && \
    cd csmith && \
    ./configure && \
    cd .. && \
    make -j && ls

VOLUME /rigel/compilers

CMD ["/rigel/Rigel"]