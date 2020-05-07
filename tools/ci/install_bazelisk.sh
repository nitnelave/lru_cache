#!/usr/bin/env bash
# Download bazelisk for a given version and platform.
set -euo pipefail

_file_name() {
  case "$1" in
  macos-latest)
    echo "bazelisk-darwin-amd64"
    ;;
  ubuntu-latest)
    echo "bazelisk-linux-amd64"
    ;;
  windows-latest)
    echo "bazelisk-windows-amd64.exe"
    ;;
  *)
    echo "Invalid platform" 1>&2
    exit 1
    ;;
  esac
}

_release_link() {
  echo "https://github.com/$1/releases/download/$2/$3"
}

install_bazelisk() {
  local version="$1"
  local platform="$2"

  file="$(_file_name "$platform")"
  link="$(_release_link "bazelbuild/bazelisk" "$version" "$file")"
  bazel_path="bin/bazel"
  bazel_path_dir="$(dirname "$bazel_path")"

  set -x
  curl -LO "$link"
  mkdir -p "$bazel_path_dir"
  mv "${file}" "$bazel_path"
  chmod +x "$bazel_path"
}

version="$1"
platform="$2"
install_bazelisk "$version" "$platform"
cp tools/ci/bazel.rc "$HOME/.bazelrc"
