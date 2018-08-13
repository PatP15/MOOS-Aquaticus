#!/bin/bash

# ===============================
# Get input from user
# ===============================

printf "CID: "
read cid
cid=$(printf "%03s" $cid)

printf "oceanai username: "
read username

# ===============================
# Get input from user
# ===============================

printf "Download logs in this directory [y/N]: "
read response

if [[ "$response" != "y" && "$response" != "Y" && "$response" != "yes" ]]; then
    exit 0
fi

# ===============================
# Download files
# ===============================

mkdir -p raw_logs

printf "===============================\n"
printf " Downloading logs from C${cid}\n" 
printf "===============================\n"

rsync -rv --progress ${username}@oceanai.mit.edu:'/raiddrive/aquaticus_data/*/C'${cid}'*' raw_logs

# ===============================
# Untar files
# ===============================

# Untar tar files
for file in raw_logs/C${cid}*.tar; do
    tar -xvf $file -C raw_logs/.
done

# Untar tgz files
for file in raw_logs/C${cid}*.tgz; do
    tar -xvzf $file -C raw_logs/.
done

# ===============================
# Create file structure
# ===============================

printf "link logs in this directory [y/n]: "
read response

if [[ "$response" != "y" && "$response" != "y" && "$response" != "yes" ]]; then
    exit 0
fi

printf "===============================\n"
printf " Linking logs from C${cid}\n" 
printf "===============================\n"

bllinks.sh --log_dir=raw_logs --cid="$cid"
