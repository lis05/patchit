#!/usr/bin/env bash
#
# Copyright 2014 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# Generate version information
if ghash=$(git rev-parse --short=12 --verify HEAD 2>/dev/null); then
  numcommits=$(git rev-list HEAD | wc -l)
  ver="v0.${numcommits}-${ghash}"
else
  ver="unknown"
fi

echo "$ver"
