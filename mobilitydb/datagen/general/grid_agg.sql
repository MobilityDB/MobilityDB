DO $$

DECLARE 
  NROWS int = 200;
  NCOLS int = 200;
  elevation float[];
  grid int[];
  resultgrid float[];
  pos int;
  id_v int;
  r_v int;
  c_v int;
  radius_v int;
  db_v float;
  r1 int;
  c1 int;
  no_positions int = 0;
  no_positions_radius int = 0;
  pbbox_cursor CURSOR FOR
      SELECT id, r, c, radius, db
      FROM PointBoundingBox;
  pbbox_record RECORD;
BEGIN

  elevation  = array_fill(NULL::float, array[NROWS * NCOLS]);
  grid  = array_fill(NULL::int, array[NROWS * NCOLS]);
  resultgrid  = array_fill(NULL::float, array[NROWS * NCOLS]);

  FOR x IN 1..NROWS * NCOLS
  LOOP
    elevation[x] = random_float(1, 10);
    resultgrid[x] = 0;
  END LOOP;
  
  -- FOR r IN 1..NROWS LOOP
    -- FOR c IN 1..NCOLS LOOP
      -- pos = (r - 1) * NCOLS + c;
      -- RAISE notice '[%,%] -> %',r, c, pos;
    -- END LOOP;
  -- END LOOP;

  -- Open cursor
  OPEN pbbox_cursor;
  -- Fetch rows and return
  RAISE notice 'Start reading records <----------';
  LOOP
      FETCH NEXT FROM pbbox_cursor INTO pbbox_record;
      EXIT WHEN NOT FOUND;
      id_v = pbbox_record.id;
      r_v = pbbox_record.r;
      c_v = pbbox_record.c;
      radius_v = pbbox_record.radius;
      db_v = pbbox_record.db;
      no_positions = no_positions + 1;
      IF no_positions % 1e5 = 0 THEN
        RAISE notice 'Record No: %', no_positions;
      END IF;
      FOR r1 IN r_v - radius_v .. r_v + radius_v LOOP
        FOR c1 IN c_v - radius_v .. c_v + radius_v LOOP
          pos = (r1 - 1) * NCOLS + c1;
          grid[pos] = grid[pos] + 1;
          resultgrid[pos] = resultgrid[pos] + db_v * elevation[pos];
          no_positions_radius = no_positions_radius + 1;
        END LOOP;
      END LOOP;

  END LOOP;
  RAISE notice '% records read <----------', no_positions;
  RAISE notice '% grid updates <----------', no_positions_radius;
  -- Close cursor
  CLOSE pbbox_cursor;

  RAISE notice 'Writing the Noise grid into Table CompNoise <----------';

  DROP TABLE IF EXISTS CompNoise;
  CREATE TABLE CompNoise(
      grid_r integer,
      grid_c integer,
      noise float,
      npoints integer
  );

  FOR r IN 1..NROWS LOOP
    FOR c IN 1..NCOLS LOOP
      pos = (r - 1) * NCOLS + c;
      INSERT INTO CompNoise(grid_r, grid_c, noise, npoints)
      VALUES (r1, c1, resultgrid[pos], grid[pos]);
    END LOOP;
  END LOOP;

END $$;