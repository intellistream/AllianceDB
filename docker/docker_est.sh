docker build . -t ooojoin:latest
docker cp sshd_config ooojoin:/etc/ssh/
docker run  --name="ooojoin" -h OoOJoin -it ooojoin

