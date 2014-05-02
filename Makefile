all: \
	makefile            \
	crop-area           \
	get-osm-area        \
	parse-osm           \
	new-york-latest.osm \
	ny.txt              \
	b1-thiels.png       \
	b2-sloatsburg.png   \
	c2-monroe.png       \
	c1-popolopen.png


# create local makefile for build directory

Makefile:
	[ ! -f ../trails-build/makefile ] && echo -e "include ../trails/makefile" > ../trails-build/makefile

# build programs

%.o: ../trails/%.c
	cc -Werror -I../trails -I${HOME}/local/include -I${HOME}/local/include/libxml2 -g -c $< -o $@

parse-osm: parse-osm.o coordconv.o std_hash.o osm-parsing.o tfw.o
	cc -g $^ -Wl,-rpath=${HOME}/local/lib -L${HOME}/local/lib -lproj -lxml2 -o $@

crop-area: crop-area.o
	cc -g $^ -o $@

get-osm-area: get-osm-area.o coordconv.o std_hash.o tfw.o
	cc -g $^ -Wl,-rpath=${HOME}/local/lib -L${HOME}/local/lib -lproj -lexpat -o $@

#  download zipped pdf maps from USGS

gda_5940193.zip:
	wget -O $@ http://ims.er.usgs.gov/gda_services/download?item_id=5940193

gda_5940195.zip:
	wget -O $@ http://ims.er.usgs.gov/gda_services/download?item_id=5940195

gda_5940185.zip:
	wget -O $@ http://ims.er.usgs.gov/gda_services/download?item_id=5940185

gda_5940261.zip:
	wget -O $@ http://ims.er.usgs.gov/gda_services/download?item_id=5940261

gda_5939759.zip:
	wget -O $@ http://ims.er.usgs.gov/gda_services/download?item_id=5939759

#  unzip pdf maps

o41074b2.pdf: gda_5940193.zip
	unzip $<
	mv NY_Sloatsburg_20130319_TM_geo.pdf $@
	touch $@

o41074b1.pdf: gda_5940195.zip
	unzip $<
	mv NY_Thiells_20130319_TM_geo.pdf $@
	touch $@

o41074c1.pdf: gda_5940185.zip
	unzip $<
	mv NY_Popolopen_Lake_20130319_TM_geo.pdf $@
	touch $@

o41074c2.pdf: gda_5940261.zip
	unzip $<
	mv NY_Monroe_20130321_TM_geo.pdf $@
	touch $@
	
o41073c8.pdf: gda_5939759.zip
	unzip $<
	mv NY_Peekskill_20130315_TM_geo.pdf $@
	touch $@

#  convert pdf maps to png

o41074b2.png: o41074b2.pdf
	convert -density 300x300 $< $@

o41074b1.png: o41074b1.pdf
	convert -density 300x300 $< $@

o41074c1.png: o41074c1.pdf
	convert -density 300x300 $< $@

o41074c2.png: o41074c2.pdf
	convert -density 300x300 $< $@

o41073c8.png: o41073c8.pdf
	convert -density 300x300 $< $@

# download OpenStreetMap data

o41074a2.osm:
	wget -O $@ http://api.openstreetmap.org/api/0.6/map?bbox=-74.25,41.100,-74.125,41.125

o41074b1.osm:
	wget -O $@ http://api.openstreetmap.org/api/0.6/map?bbox=-74.125,41.125,-74.00,41.25

o41074b2.osm:
	wget -O $@ http://api.openstreetmap.org/api/0.6/map?bbox=-74.25,41.125,-74.125,41.25

o41074b3.osm:
	wget -O $@ http://api.openstreetmap.org/api/0.6/map?bbox=-74.375,41.125,-74.25,41.25

o41074c1.osm:
	wget -O $@ http://api.openstreetmap.org/api/0.6/map?bbox=-74.125,41.25,-74.00,41.375

o41074c2.osm:
	wget -O $@ http://api.openstreetmap.org/api/0.6/map?bbox=-74.25,41.25,-74.125,41.375

new-york-latest.osm.bz2:
	wget http://download.geofabrik.de/north-america/us/new-york-latest.osm.bz2

new-york-latest.osm: new-york-latest.osm.bz2
	bunzip2 -k $<

# create tfw files using map tool
%.tfw:
	@echo starting tfw map tool for $@

# convert map data to Imagemagick mvg

o41074b2.mvg: parse-osm o41074b2.osm o41074b2.tfw
	./parse-osm \
		--osm-file o41074b2.osm \
		--tfw-file o41074b2.tfw \
		--log-file $(basename $@).log \
		--out-file $(basename $@).mvg.tmp
	mv $(basename $@).mvg.tmp $(basename $@).mvg

o41074b1.mvg: parse-osm o41074b1.osm o41074b1.tfw
	./parse-osm \
		--osm-file o41074b1.osm \
		--tfw-file o41074b1.tfw \
		--log-file $(basename $@).log \
		--out-file $(basename $@).mvg.tmp
	mv $(basename $@).mvg.tmp $(basename $@).mvg

o41074c2.mvg: parse-osm o41074c2.osm o41074c2.tfw
	./parse-osm \
		--osm-file o41074c2.osm \
		--tfw-file o41074c2.tfw \
		--log-file $(basename $@).log \
		--out-file $(basename $@).mvg.tmp
	mv $(basename $@).mvg.tmp $(basename $@).mvg

o41074c1.mvg: parse-osm o41074c1.osm o41074c1.tfw
	./parse-osm \
		--osm-file o41074c1.osm \
		--tfw-file o41074c1.tfw \
		--log-file $(basename $@).log \
		--out-file $(basename $@).mvg.tmp
	mv $(basename $@).mvg.tmp $(basename $@).mvg

# convert generated mvg to png

b1-thiels.png: o41074b1.png o41074b1.mvg
	convert o41074b1.png -draw @o41074b1.mvg $@

b2-sloatsburg.png: o41074b2.png o41074b2.mvg
	convert o41074b2.png -draw @o41074b2.mvg $@

c2-monroe.png: o41074c2.png o41074c2.mvg
	convert o41074c2.png -draw @o41074c2.mvg $@

c1-popolopen.png: o41074c1.png o41074c1.mvg
	convert o41074c1.png -draw @o41074c1.mvg $@

ny.txt: get-osm-area new-york-latest.osm
	./get-osm-area -f new-york-latest.osm > ny.txt
