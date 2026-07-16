#!/usr/bin/env bash

# -----------------------------------------------------------------------------
# DO NOT EDIT!
# Automatically generated from npm-packages-helper/templates/*.
#
# This file is part of the µOS++ project (http://micro-os-plus.github.io).
# Copyright (c) 2026 Liviu Ionescu.  All rights reserved.
#
# Permission to use, copy, modify, and/or distribute this software
# for any purpose is hereby granted, under the terms of the MIT license.
#
# If a copy of the license was not distributed with this file, it can
# be obtained from https://opensource.org/licenses/mit.
#
# -----------------------------------------------------------------------------

# -----------------------------------------------------------------------------
# Safety settings (see https://gist.github.com/ilg-ul/383869cbb01f61a51c4d).

if [[ ! -z ${DEBUG} ]]
then
  set ${DEBUG} # Activate the expand mode if DEBUG is anything but empty.
else
  DEBUG=""
fi

set -o errexit # Exit if command failed.
set -o pipefail # Exit if pipe failed.
set -o nounset # Exit if variable not set.

# Remove the initial space and instead use '\n'.
IFS=$'\n\t'

# -----------------------------------------------------------------------------
# Identify the script location, to reach, for example, the helper scripts.

script_path="$0"
if [[ "${script_path}" != /* ]]
then
  # Make relative path absolute.
  script_path="$(pwd)/$0"
fi

export script_path
export script_name="$(basename "${script_path}")"

export script_folder_path="$(dirname "${script_path}")"
export script_folder_name="$(basename "${script_folder_path}")"

# =============================================================================

argv="$@"

source "node_modules/@xpack/npm-packages-helper/maintenance-scripts/scripts-helper-source.sh"

# $1: liquid source
# $2: destination file path
function substitute()
{
  local _from_file_path="$1" # liquid source
  local _to_file_path="$2" # destination

  echo "liquidjs -> ${_to_file_path}"
  # pwd

  liquidjs \
    --context "${xcdl_context}" \
    --template "@${_from_file_path}" \
    --output "${_to_file_path}.new" \
    --strict-filters \
    --strict-variables \
    --lenient-if \

  rm -f "${_to_file_path}"
  mv "${_to_file_path}.new" "${_to_file_path}"
}

# -----------------------------------------------------------------------------

if [[ $# -ne 1 ]]
then
  echo "Usage: $(basename "$0") <xcdl-package.jsonc>"
  exit 1
fi

xcdl_jsonc_path="$1"

echo
echo "Processing ${xcdl_jsonc_path}..."

# -----------------------------------------------------------------------------

project_folder_path="$(dirname "${script_folder_path}")"

if [ ! -f "${project_folder_path}/package.json" ]
then
  echo "missing mandatory ${project_folder_path}/package.json..."
  exit 1
fi

if [ ! -f "${xcdl_jsonc_path}" ]
then
  echo "missing mandatory ${xcdl_jsonc_path}..."
  exit 1
fi

xcdl_context="{}"

serialise_string_property_to "xcdl_context" "libraryFilePath" \
      "${xcdl_jsonc_path}"

# Read in top package.json.
xpack_package_json="$(json -f "${project_folder_path}/package.json" -o json-0)"

serialise_object_property_to "xcdl_context" "package" \
  "$(echo ${xpack_package_json})" 

# Read in xcdl-package.jsonc and convert to plain JSON.
xcdl_package_json="$(json5 "${xcdl_jsonc_path}" | json -o json-0)"

serialise_object_property_to "xcdl_context" "xcdl" \
  "$(echo "${xcdl_package_json}")" 

echo
echo -n '"xcdl_context": '
echo "${xcdl_context}" | json

echo
echo "generating files..."
echo

substitute "${script_folder_path}/templates/CMakeLists-liquid.txt" "CMakeLists.txt"
substitute "${script_folder_path}/templates/meson-liquid.build" "meson.build"

echo
echo "'${script_name} ${argv}' done"

# Completed successfully.
exit 0

# -----------------------------------------------------------------------------

