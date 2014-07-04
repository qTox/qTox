@mkdir %~dp0libs
%~dp0tools\wget --no-check-certificate http://jenkins.libtoxcore.so/job/libtoxcore-win32-i686/lastSuccessfulBuild/artifact/libtoxcore-win32-i686.zip -O %~dp0libs\libtoxcore-latest.zip
%~dp0tools\unzip -o %~dp0libs\libtoxcore-latest.zip -d %~dp0libs\
@del %~dp0libs\libtoxcore-latest.zip