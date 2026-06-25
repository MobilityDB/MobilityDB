/*
 * Copyright 2024 Zacharias Knudsen
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *	   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "deprecate.h"

H3_DEPRECATE("1.0.0", h3_basecells);
H3_DEPRECATE("1.0.0", h3_h3_get_base_cell);
H3_DEPRECATE("1.0.0", h3_h3_get_resolution);
H3_DEPRECATE("1.0.0", h3_h3_indexes_are_neighbors);
H3_DEPRECATE("1.0.0", h3_h3_is_pentagon);
H3_DEPRECATE("1.0.0", h3_h3_is_res_class_iii);
H3_DEPRECATE("1.0.0", h3_h3_is_valid);
H3_DEPRECATE("1.0.0", h3_h3_set_to_linked_geo);
H3_DEPRECATE("1.0.0", h3_h3_to_children);
H3_DEPRECATE("1.0.0", h3_h3_to_geo_boundary);
H3_DEPRECATE("1.0.0", h3_h3_to_geo);
H3_DEPRECATE("1.0.0", h3_h3_to_parent);
H3_DEPRECATE("1.0.0", h3_h3_unidirectional_edge_is_valid);
H3_DEPRECATE("1.0.0", h3_haversine_distance);
H3_DEPRECATE("3.4.0", h3_degs_to_rads);
H3_DEPRECATE("3.4.0", h3_rads_to_degs);
H3_DEPRECATE("3.5.0", h3_edge_length_km);
H3_DEPRECATE("3.5.0", h3_edge_length_m);
H3_DEPRECATE("3.5.0", h3_get_unidirectional_edge_boundary);
H3_DEPRECATE("3.5.0", h3_hex_area_km2);
H3_DEPRECATE("3.5.0", h3_hex_area_m2);
H3_DEPRECATE("3.5.0", h3_hex_range_distances);
H3_DEPRECATE("3.5.0", h3_hex_range);
H3_DEPRECATE("3.5.0", h3_hex_ranges);
H3_DEPRECATE("3.6.0", h3_h3_to_string);
H3_DEPRECATE("3.6.0", h3_string_to_h3);
H3_DEPRECATE("4.0.0", h3_compact);
H3_DEPRECATE("4.0.0", h3_distance);
H3_DEPRECATE("4.0.0", h3_exact_edge_length);
H3_DEPRECATE("4.0.0", h3_experimental_h3_to_local_ij);
H3_DEPRECATE("4.0.0", h3_experimental_local_ij_to_h3);
H3_DEPRECATE("4.0.0", h3_geo_to_h3);
H3_DEPRECATE("4.0.0", h3_get_base_cell);
H3_DEPRECATE("4.0.0", h3_get_destination_h3_index_from_unidirectional_edge);
H3_DEPRECATE("4.0.0", h3_get_faces);
H3_DEPRECATE("4.0.0", h3_get_h3_indexes_from_unidirectional_edge);
H3_DEPRECATE("4.0.0", h3_get_h3_unidirectional_edge_boundary);
H3_DEPRECATE("4.0.0", h3_get_h3_unidirectional_edge);
H3_DEPRECATE("4.0.0", h3_get_h3_unidirectional_edges_from_hexagon);
H3_DEPRECATE("4.0.0", h3_get_origin_h3_index_from_unidirectional_edge);
H3_DEPRECATE("4.0.0", h3_get_pentagon_indexes);
H3_DEPRECATE("4.0.0", h3_get_res_0_indexes);
H3_DEPRECATE("4.0.0", h3_hex_area);
H3_DEPRECATE("4.0.0", h3_hex_ring);
H3_DEPRECATE("4.0.0", h3_indexes_are_neighbors);
H3_DEPRECATE("4.0.0", h3_is_valid);
H3_DEPRECATE("4.0.0", h3_k_ring_distances);
H3_DEPRECATE("4.0.0", h3_k_ring);
H3_DEPRECATE("4.0.0", h3_line);
H3_DEPRECATE("4.0.0", h3_num_hexagons);
H3_DEPRECATE("4.0.0", h3_point_dist);
H3_DEPRECATE("4.0.0", h3_polyfill);
H3_DEPRECATE("4.0.0", h3_set_to_multi_polygon);
H3_DEPRECATE("4.0.0", h3_to_center_child);
H3_DEPRECATE("4.0.0", h3_to_geo_boundary);
H3_DEPRECATE("4.0.0", h3_to_geo);
H3_DEPRECATE("4.0.0", h3_uncompact);
H3_DEPRECATE("4.0.0", h3_unidirectional_edge_is_valid);
H3_DEPRECATE("4.1.0", h3_cell_to_boundary_wkb);
H3_DEPRECATE("4.1.0", h3_cells_to_multi_polygon_wkb);

H3_SOFT_DEPRECATE(h3_vertex_to_lat_lng, h3_vertex_to_latlng);
H3_SOFT_DEPRECATE(h3_cell_to_lat_lng, h3_cell_to_latlng);
H3_SOFT_DEPRECATE(h3_lat_lng_to_cell, h3_latlng_to_cell);
