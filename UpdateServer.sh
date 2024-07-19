#!/usr/bin/env bash
sudo -i -u MotherServer bash << EOF
cd MotherServer
git stash push
git pull
git stash pop
EOF
systemctl reboot
