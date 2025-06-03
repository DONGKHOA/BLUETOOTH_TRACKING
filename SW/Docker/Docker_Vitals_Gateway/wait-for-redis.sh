#!/bin/sh
echo "Waiting for Redis..."

while ! nc -z localhost 6379; do
  sleep 0.5
done

echo "Redis is up - starting app"
exec "$@"
