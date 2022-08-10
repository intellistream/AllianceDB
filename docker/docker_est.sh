docker build . -t adb_u2204_a:latest
docker cp sshd_config aliancedb_u22_04_a:/etc/ssh/
docker run --privileged --mount source=rootfs,target=/home/sutd/project --name="aliancedb_u22_04_a" -h sutd -it adb_u2204_a

