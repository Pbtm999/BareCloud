
FROM gcc:13 AS builder

WORKDIR /app
COPY . .
RUN make

FROM ubuntu:22.04
WORKDIR /app

COPY --from=builder /app/barecloud .

COPY static/ ./static/