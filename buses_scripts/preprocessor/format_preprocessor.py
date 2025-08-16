import os
import csv
import sys
from expected_types import expected_types  # Importing the expected_types dictionary from another file

def remove_anomaly_characters(input_directory, output_directory, expected_types):
    # Create output directory if it doesn't exist
    if not os.path.exists(output_directory):
        os.makedirs(output_directory)

    # Loop through each file in the input directory
    for filename in os.listdir(input_directory):
        if filename.endswith(".txt"):  # Check if the file has a .txt extension (considering it as CSV)
            csv_file = os.path.join(input_directory, filename)  # Path to the input CSV file
            new_csv_file = os.path.join(output_directory, filename)  # Path to the output modified CSV file
            with open(csv_file, 'r') as file:
                csv_reader = csv.reader(file)
                header = next(csv_reader)  # Read the header from the CSV file

                # Remove non-alphabetic characters from the beginning of each column name in the header
                for i in range(len(header)):
                    if not header[i][0].isalpha():
                        header[i] = header[i][1:]

                anomalies = []
                file_expected_types = expected_types.get(filename, {})  # Get expected types for the current file
                for i, column_name in enumerate(header):
                    if column_name in file_expected_types and file_expected_types[column_name] in [int, float]:
                        # Detect anomalies in columns with numeric expected type that have quotation marks
                        if column_name.startswith('"') and column_name.endswith('"'):
                            column_name = column_name.strip('"')
                            anomalies.append(i)
                
                modified_rows = []
                for row in csv_reader:
                    modified_row = []
                    for i, value in enumerate(row):
                        modified_value = value.strip()  # Strip leading and trailing spaces from each value
                        if i in anomalies:
                            modified_value = value[1:-1]  # Remove first and last characters if anomaly is detected
                        modified_row.append(modified_value)
                    modified_rows.append(modified_row)
                
                # Write the modified CSV data to the new CSV file
                with open(new_csv_file, 'w', newline='') as new_file:
                    csv_writer = csv.writer(new_file)
                    csv_writer.writerow(header)
                    csv_writer.writerows(modified_rows)
                
                print("Anomaly characters have been successfully removed for", filename)

if __name__ == "__main__":
    # Vérification des arguments de ligne de commande
    if len(sys.argv) != 3:
        print("Usage: python3 preprocess.py input_directory output_directory")
    else:
        input_directory = sys.argv[1]
        output_directory = sys.argv[2]

        if not os.path.isdir(input_directory):
            print("Error: The input directory is not a valid directory.")
            sys.exit(1)
        
        remove_anomaly_characters(input_directory, output_directory, expected_types)