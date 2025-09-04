# GTFS Static


To import GTFS static data in MobilityDB, we use SQL statements for the conversion, but we can use a Python script or another library to preprocess our data.

## Requirements
- MobilityDB >= 1.0
- (Optional) [<tt>pfaedle</tt>](https://www.genome.gov/) if <code>shapes.txt</code> is missing in the dataset.
- (Optional) Python >= 3.0 if a preprocessing step is needed.

## User guide

If some steps do not work for format or missing shapes, see further sections.

- Create the necessary tables to host the data by running <code>create_tables.sql</code>



* Import the data into PostgreSQL/PostGIS with <code>import_sql.sql</code>; Uncomment and replace the call to the function with the path to your extracted GTFS folder.
* (Optional) If needed, run <code>preprocess_target.sql</code> to reduce the scope of the studied area, be sure to change the coordinates with the correct area.
* Finally, we transform the data into MobilityDB by running <code>transform_mdb.sql</code>. This operation can take time, depending on your dataset size. A commented block is available to reduce the feed to a given time range. 


Here is a visualisation of all the public transport vehicles in Brussels on 21st July 2023 in the morning. The data are imported from GTFS Static feeds and visualised with MOVE on QGIS.
![](./img/brussels_static.gif)


### (Optional) Format preprocessing

If the SQL import step fails due to a format problem, run the preprocessor written in Python, which will try to fix the formatting anomalies.

```
python3 preprocessor/format_preprocessor.py input_gtfs/ output_gtfs/
```

It is possible to specify the output directory name.

### (Optional) Missing Shapes Generation

The success of our scripts depend on the presence of many files, one of them is optional. In the case where <code>shapes.txt</code> is missing, generate it by using <tt>pfaedle</tt>. 

Be sure to get an OSM file containing all the ways corresponding to the means of transport detailed in your GTFS feed. Example, the NMBS/SNCB in Belgium feed contains trains and buses. So we need an OSM file containing both. Here's the corresponding OSM query. 

```
[out:xml] [timeout:25];
 {{geocodeArea:Belgium}} -> .area_0;
(
    node["route"="railway"](area.area_0);
    node["route"="bus"](area.area_0);
    way["route"="railway"](area.area_0);
    way["route"="bus"](area.area_0);
    relation["route"="railway"](area.area_0);
    relation["route"="bus"](area.area_0);
);
(._;>;);
out body;
```

With the exported OSM file and your initial GTFS dataset, you can now run <tt>pfaedle</tt> to generate the missing <code>shapes.txt</code> file.

```
pfaedle -x export.osm unzipped_gtfs_dir/
```

