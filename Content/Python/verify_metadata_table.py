import unreal

ASSET_PATH = "/Game/Generated/Attributes/PlayerCoreAttributesMetadata"

logger = unreal.log
error = unreal.log_error

metadata_table = unreal.load_asset(ASSET_PATH, unreal.DataTable)
if metadata_table is None:
    error(f"Failed to load DataTable asset at {ASSET_PATH}")
    raise SystemExit(1)

row_names = unreal.DataTableFunctionLibrary.get_data_table_row_names(metadata_table)
if not row_names:
    error("DataTable has no rows")
    raise SystemExit(2)

logger(f"Loaded DataTable: {ASSET_PATH}")
logger(f"Row Count: {len(row_names)}")
logger(f"DataTableFunctionLibrary members: {dir(unreal.DataTableFunctionLibrary)}")

json_string = unreal.DataTableFunctionLibrary.export_data_table_to_json_string(metadata_table)
if not json_string:
    error("Failed to export DataTable to JSON")
    raise SystemExit(3)

import json

try:
    rows = json.loads(json_string)
except json.JSONDecodeError as exc:
    error(f"Failed to parse DataTable JSON: {exc}")
    raise SystemExit(4)

for entry in rows:
    row_name = entry.get("Name") or entry.get("RowName") or "<Unknown>"
    data = entry.get("RowStruct", {})
    base = data.get("BaseValue")
    min_value = data.get("MinValue")
    max_value = data.get("MaxValue")
    description = data.get("Description")
    logger(f"Row '{row_name}': BaseValue={base}, MinValue={min_value}, MaxValue={max_value}, Description='{description}'")

logger("Metadata table verification complete.")
