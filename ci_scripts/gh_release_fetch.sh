# gh_release_fetch.sh — sourced by the fetch_*.sh dependency scripts.
#
#   gh_release_fetch <owner/repo> <tag> <asset> <outfile>
#
# Downloads one GitHub release asset. Tries the normal
# github.com/.../releases/download/ URL first, then falls back to the REST API
# asset endpoint (api.github.com/repos/.../releases/assets/<id>). The two are
# served by different front ends: the download URL has gone down with 504s for
# every repo while the API endpoint kept serving the same files, and a plain
# `curl --retry 3` gives up after ~8 seconds. Returns non-zero only when both fail.
# GITHUB_TOKEN / GH_TOKEN, if set, authenticates the API calls (rate limits).

gh_release_fetch() {
    _ghr_repo="$1"; _ghr_tag="$2"; _ghr_asset="$3"; _ghr_out="$4"
    _ghr_url="https://github.com/${_ghr_repo}/releases/download/${_ghr_tag}/${_ghr_asset}"

    echo "gh_release_fetch: downloading $_ghr_url"
    if curl -fL --retry 4 --retry-delay 5 --connect-timeout 30 -o "$_ghr_out" "$_ghr_url"; then
        return 0
    fi

    echo "gh_release_fetch: download URL failed, trying the GitHub API asset endpoint" >&2
    _ghr_token="${GITHUB_TOKEN:-${GH_TOKEN:-}}"
    set -- -fsSL --retry 4 --retry-delay 5 --connect-timeout 30
    [ -n "$_ghr_token" ] && set -- "$@" -H "Authorization: Bearer $_ghr_token"

    # Release JSON lists each asset as "url": ".../releases/assets/<id>" followed
    # by its "name"; pair them up without needing jq or python.
    _ghr_api="$(curl "$@" -H "Accept: application/vnd.github+json" \
        "https://api.github.com/repos/${_ghr_repo}/releases/tags/${_ghr_tag}")" || return 1
    _ghr_asset_url="$(printf '%s\n' "$_ghr_api" | tr ',' '\n' | awk -v want="$_ghr_asset" '
        /"url": *"https:\/\/api\.github\.com\/repos\/.*\/releases\/assets\/[0-9]+"/ {
            u = $0; sub(/.*"url": *"/, "", u); sub(/".*/, "", u); next
        }
        /"name": *"/ {
            n = $0; sub(/.*"name": *"/, "", n); sub(/".*/, "", n)
            if (n == want && u != "") { print u; exit }
        }')"
    if [ -z "$_ghr_asset_url" ]; then
        echo "gh_release_fetch: asset $_ghr_asset not found in release $_ghr_tag of $_ghr_repo" >&2
        return 1
    fi
    echo "gh_release_fetch: downloading $_ghr_asset_url"
    curl "$@" -H "Accept: application/octet-stream" -o "$_ghr_out" "$_ghr_asset_url"
}
