FROM --platform=linux/amd64 ubuntu:22.04 as builder

RUN apt-get update
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y build-essential cmake libgmp-dev libmpfr-dev

COPY . /repo
WORKDIR /repo/build
RUN cmake ..
RUN make -j8

FROM ubuntu:22.04 as package
RUN apt-get update
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y libmpfr-dev
COPY --from=builder /repo/build/bin/feral /
COPY --from=builder /repo/build/lib/* /deps/
ENV LD_LIBRARY_PATH=/deps
