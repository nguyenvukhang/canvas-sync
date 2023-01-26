#!/usr/bin/env bash

latest_tag() {
  git for-each-ref --sort=-v:refname \
    --count=1 \
    'refs/tags/v[0-9]*.[0-9]*.[0-9]*' \
    --format="%(refname:short)"
}

latest_tag
