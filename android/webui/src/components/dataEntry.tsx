import { Stack, Switch, TextField } from "@mui/material";
import React from "react"
import "./css/data-entry.css"
export default function DataEntry(props) {
    let data;
    switch (props.type) {
        case "text-edit": {
            data = <TextField className="data" variant="outlined" value={props.data}> </TextField>;
            break
        }
        case "number-edit": {
            data = <TextField className="data" variant="outlined" value={props.data} inputProps={{ type: "number" }}> </TextField>;
            break
        }
        case "switch":{
            data =<Switch></Switch>;
            break;{/* todo: to improve */}
        }
        default: {
            data = <span className="data">{props.data}</span>;
        }
    }
    return (
        <Stack className="data-entry" direction="row" style={{ justifyContent: "space-between" }}>
            <span className="label">{props.label}</span>
            {data}
        </Stack>

    )
}
