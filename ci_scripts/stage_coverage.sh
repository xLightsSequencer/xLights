#!/bin/sh
# Put the unit test coverage report into a dashboard site before it is built.
#
#   stage_coverage.sh SITE_DIR SITE_URL [TARBALL]
#
# TARBALL is this run's report (coverage.tar.gz from the nightly's coverage
# job). Without one -- the coverage build failed, or this is a republish for
# the Xcode Cloud results -- the report the site already publishes is fetched
# from SITE_URL, since a Pages deploy replaces the whole site. The tarball holds
# coverage/ and is published as well, for the next run to carry over. A missing
# report is not an error: the dashboard is built without it.
SITE="$1"
URL="$2"
TARBALL="$3"
mkdir -p "$SITE"
if [ -z "$TARBALL" ] || [ ! -s "$TARBALL" ]; then
    TARBALL="$(mktemp)"
    if ! curl -fsSL "$URL/coverage.tar.gz?t=$(date +%s)" -H 'Cache-Control: no-cache' -o "$TARBALL"; then
        echo "no coverage report to publish"
        exit 0
    fi
    echo "carrying over the published coverage report"
fi
if tar xzf "$TARBALL" -C "$SITE" coverage; then
    cp "$TARBALL" "$SITE/coverage.tar.gz"
else
    echo "unreadable coverage report, publishing without it"
    rm -rf "$SITE/coverage"
fi
exit 0
