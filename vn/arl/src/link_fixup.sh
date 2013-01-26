#/bin/sh

cd /usr/lib

BOOST_NAME="boost_system boost_filesystem boost_program_options boost_regex boost_date_time"

for NAME in ${BOOST_NAME}; do
	test ! -e lib${NAME}-gcc42-mt.a &&
		(echo -n "${NAME} " && 
		 ln -s lib${NAME}-mt.a lib${NAME}-gcc42-mt.a &&
		 ln -s lib${NAME}-mt.so lib${NAME}-gcc42-mt.so &&
		 echo "Success")
done;
