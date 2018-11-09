#!/bin/bash
set -eu

base_url='http://yann.lecun.com/exdb/mnist'
file_names=(
  'train-images-idx3-ubyte'
  'train-labels-idx1-ubyte'
  't10k-images-idx3-ubyte'
  't10k-labels-idx1-ubyte'
)

for file_name in ${file_names[@]}; do
  echo "Retrieving $file_name"
  wget "$base_url/$file_name.gz"
  gunzip -d "$file_name.gz"
done
