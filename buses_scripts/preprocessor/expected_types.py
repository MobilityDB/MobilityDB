expected_types = {
    'agency.txt': {
        'agency_id': str,
        'agency_name': str,
        'agency_url': str,
        'agency_timezone': str,
        'agency_lang': str,
    },
    'stops.txt': {
        'stop_id': str,
        'stop_name': str,
        'stop_lat': float,
        'stop_lon': float,
        'location_type': int
    },
    'routes.txt': {
        'route_id': str,
        'agency_id': str,
        'route_short_name': str,
        'route_long_name': str,
        'route_type': int
    },
    'trips.txt': {
        'route_id': str,
        'service_id': str,
        'trip_id': str,
        'direction_id': int
    },
    'stop_times.txt': {
        'trip_id': str,
        'arrival_time': str,
        'departure_time': str,
        'stop_id': str,
        'stop_sequence': int,
        'stop_headsign' : str,
        'pickup_type' : int,
        'drop_off_type' : int,
        'continuous_pickup' : int,
        'continuous_drop_off' : int,
        'shape_dist_traveled':int,
        'timepoint' : int
    },
    'transfers': {
        'from_stop_id' : str,
        'to_stop_id' : str,
        'transfer_type' : int,
        'min_transfer_time' : int
    }
}