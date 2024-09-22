import csv
import json
import os

def convert_csv_to_json(fight_states_csv, fight_times_csv, initial_state_csv, fight_strategy_csvs, json_file):
    fight_states = {}
    fight_times = []
    initial_state = {
        "fightTimes": [],
        "timeRemaining": None,
        "fightStateId": None
    }
    fight_strategies = {}

    # Read the fight states CSV file
    with open(fight_states_csv, 'r') as file:
        csv_reader = csv.DictReader(file)
        for row in csv_reader:
            state_id = int(row['id'])
            name = row['name']
            fight_id = int(row['fightId'])
            
            # Create the state if it doesn't exist
            if state_id not in fight_states:
                fight_states[state_id] = {
                    "id": state_id,
                    "name": name,
                    "fightOptions": []
                }
            
            # Create fightOption
            fight_option = {"fightId": fight_id}
            if row['winStateId']:
                fight_option["winStateId"] = int(row['winStateId'])
            if row['loseStateId']:
                fight_option["loseStateId"] = int(row['loseStateId'])
                
            # Append the fightOption to the current state
            fight_states[state_id]["fightOptions"].append(fight_option)

    # Read the fight times CSV file
    with open(fight_times_csv, 'r') as file:
        csv_reader = csv.DictReader(file)
        for row in csv_reader:
            fight_time_entry = {"id": int(row['id']), "name": row['name']}
            
            # Optional fields
            if row['resetToNextFight']:
                fight_time_entry["resetToNextFight"] = int(row['resetToNextFight'])
            if row['resetToResetWithAttempt']:
                fight_time_entry["resetToResetWithAttempt"] = int(row['resetToResetWithAttempt'])
            if row['resetToResetWithWin']:
                fight_time_entry["resetToResetWithWin"] = int(row['resetToResetWithWin'])
            if row['fightStartToResetWithAttempt']:
                fight_time_entry["fightStartToResetWithAttempt"] = int(row['fightStartToResetWithAttempt'])
            if row['fightStartToResetWithWin']:
                fight_time_entry["fightStartToResetWithWin"] = int(row['fightStartToResetWithWin'])
            if row['fightStartToNextFight']:
                fight_time_entry["fightStartToNextFight"] = int(row['fightStartToNextFight'])
            if row['loss']:
                fight_time_entry["loss"] = int(row['loss'])
            
            # Append to fightTimes list
            fight_times.append(fight_time_entry)

    # Read the initial state CSV file
    with open(initial_state_csv, 'r') as file:
        csv_reader = csv.DictReader(file)
        for row in csv_reader:
            initial_state["fightTimes"].append(int(row['fightTimes']))
            if row['fightStateId']:
                initial_state["fightStateId"] = int(row['fightStateId'])
            if row['timeRemaining']:
                initial_state["timeRemaining"] = int(row['timeRemaining'])


    for csv_file in fight_strategy_csvs:
        with open('config_files/strategies/' + csv_file, 'r') as file:
            csv_reader = csv.DictReader(file)
            name = csv_file.split('.')[0]
            
            fight_strategies[name] = {
                "name": name,
                "fightId": None,
                "outcomes": []
            }
            fight_id_set = False
            for row in csv_reader:
                if not fight_id_set:
                    fight_id = int(row['fightId'])
                    fight_strategies[name]["fightId"] = fight_id
                    fight_id_set = True
                outcome = {"probability": float(row['probability']), "fightTime": int(row['fightTime'])}
                fight_strategies[name]["outcomes"].append(outcome)


    output_data = {
        "fightStates": list(fight_states.values()),
        "fightTimes": fight_times,
        "initialState": initial_state,
        "fightStrategies": list(fight_strategies.values())
    }

    # Write the data to a JSON file
    with open(json_file, 'w') as file:
        json.dump(output_data, file, indent=2)

def list_filenames_in_folder(folder_path):
    try:
        # List all files in the specified folder
        filenames = os.listdir(folder_path)
        
        # Filter out directories, keeping only files
        file_list = [f for f in filenames if os.path.isfile(os.path.join(folder_path, f))]
        
        return file_list
    except Exception as e:
        print(f"An error occurred: {e}")
        return []


folder_path = 'config_files/strategies'
fight_strategy_files = list_filenames_in_folder(folder_path)
convert_csv_to_json('config_files/fight_states.csv', 'config_files/fight_times.csv', 'config_files/initial_state.csv', fight_strategy_files, 'config.json')
