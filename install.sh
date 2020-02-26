#!/bin/bash --
# Installs Stud.IP, Vips, Courseware and LearningNet.

### Preparations
STUDIP_DIR=/opt/studip
PLUGIN_DIR="$STUDIP_DIR"/public/plugins_packages
COURSEWARE_DIR="$PLUGIN_DIR"/virtUOS/Courseware
VIPS_DIR="$PLUGIN_DIR"/virtUOS/VipsPlugin
LEARNINGNET_DIR="$PLUGIN_DIR"/virtUOS/LearningNet

PLUGIN_URL=localhost/studip/dispatch.php/admin/plugin/unregistered
REGISTER_URL=localhost/studip/dispatch.php/admin/plugin/
COURSE_URL=localhost/studip/dispatch.php/course/plus/index?cid=a07535cf2f8a72df33c12ddfa4b53dde

PHP_INI=/etc/php/7.0/apache2/php.ini
APACHE_DIR=/etc/apache2
USER="$@"
USER_DIR=/home/"$USER"
WORK_DIR="$USER_DIR"/studip-related

# exit when any command fails
set -e
# keep track of the last executed command
trap 'last_command=$current_command; current_command=$BASH_COMMAND' DEBUG
# echo an error message before exiting
trap 'echo "\"${last_command}\" command failed with exit code $?."' EXIT

# Call with root!
if [[ $EUID -ne 0 ]]; then
  exec sudo /bin/bash "$0" "$(whoami)"
else
  if [[ -z "$USER" ]]; then
    echo "Pass the developeing user as a param, e.g. 'sudo ./install.sh peter'."
  fi
fi

## Install needed packages
apt update
apt -y upgrade
add-apt-repository -y ppa:ondrej/php # We need the old PHP 7.0, get it via PPA.
apt -y install \
  php7.0 php7.0-mysql php7.0-curl php7.0-bcmath php7.0-zip php7.0-xsl \
  php7.0-xml php7.0-ldap php7.0-mbstring \
  apache2 mariadb-server \
  git curl \
  composer nodejs npm \
  cmake doxygen graphviz

# Bonus:
# "Für das Export-Tool [...] müssen der Formating Objects Processor und ein Java
# Runtime Environment installiert sein."

### PHP 7.0 and Apache2 configuration
cat << EOF >> /etc/apache2/conf-available/studip-httpd.conf
# Config for Stud.IP
Alias /studip /opt/studip/public
<Directory "/opt/studip/public">
    # für rewrite wird die Option FollowSymLinks oder SymLinksIfOwnerMatch benötigt ...
    #Options SymLinksIfOwnerMatch
    #RewriteEngine on
    #RewriteRule ^download/(normal|force_download|zip)/([0-467])/([^/]+)/(.+)$ sendfile.php?$1=1&type=$2&file_id=$3&file_name=$4 [L]
    #RewriteRule ^download/(normal|force_download|zip)/5/([^/]+)/([^/]+)/(.+)$ sendfile.php?$1=1&type=5&range_id=$2&list_id=$3&file_name=$4 [L]
    #bzw. bei Verwendung von Alias:
    #RewriteEngine on
    #RewriteBase /opt/studip/public
    #RewriteRule ^download/(normal|force_download|zip)/([0-467])/([^/]+)/(.+)$ /studip/sendfile.php?$1=1&type=$2&file_id=$3&file_name=$4 [L]
    #RewriteRule ^download/(normal|force_download|zip)/5/([^/]+)/([^/]+)/(.+)$ /studip/sendfile.php?$1=1&type=5&range_id=$2&list_id=$3&file_name=$4 [L]

    #Apache 2.2
    #Order Allow,Deny
    #Allow from all

    #Apache 2.4
    Require all granted

    php_value upload_max_filesize 7M
    php_value post_max_size 8M
    php_value memory_limit 64M
    php_value max_execution_time 300
    php_flag short_open_tag On
    php_admin_flag allow_url_fopen On
    php_value max_input_vars 10000
    # PHP Konstanten sind hier nicht verfügbar
    # 22519 = E_ALL & ~(E_NOTICE|E_DEPRECATED) PHP 5.3.x
    php_value error_reporting 22519

    # PHP 5.5 (ab 5.6 Standardeinstellung)
    php_value default_charset utf-8
    php_value mbstring.internal_encoding utf-8
</Directory>
EOF

cat << EOF >> /etc/apache2/conf-available/php70_module.conf
<IfModule dir_module>
	<IfModule php7_module>
		DirectoryIndex index.php index.html
		<FilesMatch "\.php$">
			SetHandler application/x-httpd-php
		</FilesMatch>
		<FilesMatch "\.phps$">
			SetHandler application/x-httpd-php-source
		</FilesMatch>
	</IfModule>
</IfModule>
EOF

a2enconf studip-httpd
a2enconf php70_module
systemctl reload apache2

### Stud.IP
# download studip, unzip
sudo -u "$USER" mkdir -p "$WORK_DIR"
cd "$WORK_DIR"
sudo -u "$USER" curl -o studip.zip -L https://downloads.sourceforge.net/project/studip/Stud.IP/4.4/studip-4.4.1.zip?ts=1582561420
sudo -u "$USER" unzip studip.zip
mv 4.4.1/ "$STUDIP_DIR"

# create studip database with user 'studip', password 'LearningNetPW'
mysql -u root --password="" -e "CREATE DATABASE studip DEFAULT CHARACTER SET latin1 COLLATE latin1_german1_ci"
mysql -u root --password="" -e "GRANT USAGE ON *.* TO 'studip'@'localhost' IDENTIFIED BY 'LearningNetPW'"
mysql -u root --password="" -e "GRANT SELECT, INSERT, UPDATE, DELETE, CREATE, DROP, INDEX, ALTER, CREATE TEMPORARY TABLES ON studip.* TO 'studip'@'localhost'"

# load example mysql dumps
# users: root@studip, test_dozent, test_admin, test_tutor, test_autor
# password always: testing
# CHANGE PASSWORD BEFORE THE INSTALLATION GOES INTO PRODUCTION!
mysql -u root --password="" studip < "$STUDIP_DIR"/db/studip.sql
mysql -u root --password="" studip < "$STUDIP_DIR"/db/studip_root_user.sql
mysql -u root --password="" studip < "$STUDIP_DIR"/db/studip_default_data.sql
mysql -u root --password="" studip < "$STUDIP_DIR"/db/studip_resources_default_data.sql
# comment out broken demo data
sed -i -e 's/^REPLACE INTO `abschluss`/-- REPLACE INTO `abschluss`/' \
  "$STUDIP_DIR"/db/studip_demo_data.sql
mysql -u root --password="" studip < "$STUDIP_DIR"/db/studip_demo_data.sql
mysql -u root --password="" studip < "$STUDIP_DIR"/db/studip_resources_demo_data.sql

# set db user and password in config
cp "$STUDIP_DIR"/config/config_local.inc.php.dist "$STUDIP_DIR"/config/config_local.inc.php
sed -i -e 's/DB_STUDIP_USER = ""/DB_STUDIP_USER = "studip"/' \
  "$STUDIP_DIR"/config/config_local.inc.php
sed -i -e 's/DB_STUDIP_PASSWORD = ""/DB_STUDIP_PASSWORD = "LearningNetPW"/' \
  "$STUDIP_DIR"/config/config_local.inc.php
cp "$STUDIP_DIR"/config/config.inc.php.dist "$STUDIP_DIR"/config/config.inc.php
chown -R www-data "$STUDIP_DIR"

### Composer
# composer can also be installed via apt, but that one is completely broken somehow
cd "$WORK_DIR"
EXPECTED_CHECKSUM="$(wget -q -O - https://composer.github.io/installer.sig)"
php -r "copy('https://getcomposer.org/installer', 'composer-setup.php');"
ACTUAL_CHECKSUM="$(php -r "echo hash_file('sha384', 'composer-setup.php');")"

if [ "$EXPECTED_CHECKSUM" != "$ACTUAL_CHECKSUM" ]; then
  >&2 echo 'ERROR: Invalid installer checksum'
  rm composer-setup.php
  exit 1
fi

php composer-setup.php --quiet
rm composer-setup.php
sudo -u "$USER" mkdir -p "$USER_DIR"/.local/bin
sudo -u "$USER" mv composer.phar "$USER_DIR"/.local/bin/composer
PATH=$PATH:"$USER_DIR"/.local/bin
echo 'PATH=$PATH:~/.local/bin' >> "$USER_DIR"/.profile

### Courseware
cd "$WORK_DIR"
sudo -u "$USER" git clone https://github.com/virtUOS/courseware.git
mkdir -p "$COURSEWARE_DIR"
mv -T courseware "$COURSEWARE_DIR"

cd "$COURSEWARE_DIR"
chown -R "$USER" "$COURSEWARE_DIR"
sudo -u "$USER" composer install
sudo -u "$USER" composer update
sudo -u "$USER" npm install
sudo -u "$USER" npm run build:dev

### ViPs
cd "$WORK_DIR"
sudo -u "$USER" curl -o vips.zip -L https://develop.studip.de/studip/plugins.php/pluginmarket/presenting/download/8fb3b533969d7100dc0fec4587818665
sudo -u "$USER" unzip vips.zip -d vips
mkdir -p "$VIPS_DIR"
mv -T vips "$VIPS_DIR"

### LearningNet
cd "$WORK_DIR"
sudo -u "$USER" git clone https://github.com/milsen/LearningNet.git
mkdir -p "$LEARNINGNET_DIR"
mv -T LearningNet "$LEARNINGNET_DIR"

# core and frontend
cd "$LEARNINGNET_DIR"
chown -R "$USER" "$LEARNINGNET_DIR"
sudo -u "$USER" composer install
sudo -u "$USER" composer update
sudo -u "$USER" npm install
sudo -u "$USER" npm run build:dev

# C++ dependencies
sudo -u "$USER" mkdir -p "$LEARNINGNET_DIR"/backend/deps/
cd "$LEARNINGNET_DIR"/backend/deps/
sudo -u "$USER" curl -o lemon.zip -L http://lemon.cs.elte.hu/hg/lemon-main/archive/tip.zip
sudo -u "$USER" curl -o rapidjson.zip -L https://github.com/Tencent/rapidjson/archive/master.zip
sudo -u "$USER" unzip lemon.zip
sudo -u "$USER" unzip rapidjson.zip
sudo -u "$USER" rm lemon.zip rapidjson.zip
sudo -u "$USER" mv lemon* lemon/
sudo -u "$USER" mv rapidjson* rapidjson/

# backend
sudo -u "$USER" mkdir -p "$LEARNINGNET_DIR"/backend/build
cd "$LEARNINGNET_DIR"/backend/build
sudo -u "$USER" cmake ..
sudo -u "$USER" make -j4
sudo -u "$USER" make doc
./test/compress_test && \
  ./test/check_test && \
  ./test/recommend_test && \
  echo "All tests succeeded!"

### The End
cat << EOF
Stud.IP users are root@studip, test_dozent, test_admin, test_tutor, test_autor.
The password is always 'testing'.
Visit $REGISTER_URL to register Courseware, ViPs and LearningNet.
Afterwards enable the plugins in the plugin management: $PLUGIN_URL
Lastly, activate the plugins for a specific seminar under "Mehr...".
EOF
sudo -u "$USER" firefox "$REGISTER_URL" &
sleep 5
sudo -u "$USER" firefox -new-tab "$PLUGIN_URL" &
sleep 5
sudo -u "$USER" firefox -new-tab "$COURSE_URL" &
