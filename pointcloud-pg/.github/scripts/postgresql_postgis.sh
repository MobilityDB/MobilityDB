#! /bin/bash

set -e

curl https://www.postgresql.org/media/keys/ACCC4CF8.asc | gpg --dearmor | sudo tee /etc/apt/trusted.gpg.d/apt.postgresql.org.gpg >/dev/null
echo "deb http://apt.postgresql.org/pub/repos/apt/ `lsb_release -cs`-pgdg main $POSTGRESQL_VERSION" |sudo tee /etc/apt/sources.list.d/pgdg.list

# RAISE priority of pgdg
cat << EOF >> ./pgdg.pref
Package: *
Pin: release o=apt.postgresql.org
Pin-Priority: 600
EOF
sudo mv ./pgdg.pref /etc/apt/preferences.d/
sudo apt update
sudo apt-get update
sudo apt-get purge postgresql-*
sudo apt-get install -q postgresql-$POSTGRESQL_VERSION postgresql-server-dev-$POSTGRESQL_VERSION postgresql-client-$POSTGRESQL_VERSION libcunit1-dev valgrind g++

if [ -z "$POSTGIS_VERSION" ]
then
      echo "No PostGIS version specified, skipping install of PostGIS"
else
      sudo apt-get install postgresql-$POSTGRESQL_VERSION-postgis-$POSTGIS_VERSION
fi

sudo pg_dropcluster --stop $POSTGRESQL_VERSION main
sudo rm -rf /etc/postgresql/$POSTGRESQL_VERSION /var/lib/postgresql/$POSTGRESQL_VERSION
sudo pg_createcluster -u postgres $POSTGRESQL_VERSION main --start -- --auth-local trust --auth-host trust
sudo /etc/init.d/postgresql start $POSTGRESQL_VERSION || sudo journalctl -xe
sudo -iu postgres psql -c 'CREATE ROLE runner SUPERUSER LOGIN CREATEDB;'
