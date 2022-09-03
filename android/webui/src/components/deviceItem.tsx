import { useTheme } from "@mui/material/styles"
import { ListItem, ListItemButton, ListItemIcon, ListItemText, Paper, Switch } from "@mui/material";
import { width } from "@mui/system";
import React from "react"
import { Bathroom, Bed, DeviceHub, DeviceUnknown, Fence, Kitchen, Light, Living, Microwave, Room, RoomPreferences, Tv } from "@mui/icons-material";
export function DeviceItem(props) {
    let theme = useTheme();
    let icon;
    switch (props.label) {
        case "lights": { icon = <Light />; break }
        case "stove": { icon = <Microwave />; break }
        case "microwave": { icon = <Microwave />; break }
        case "tv": { icon = <Tv />; break }
        // case "":{icon= <Fence/>;break}
        default: icon = <DeviceUnknown/>;
    }
    return (

        <Paper elevation={3} style={{ borderRadius: "5mm", margin: "5mm 3mm", overflow: "hidden" }}>
            <ListItemButton onClick={props.openDevice}>
                <ListItem
                    secondaryAction={<Switch ></Switch>}
                >

                    <ListItemIcon sx={{ minWidth: "9mm" }}>{icon}</ListItemIcon>
                    <ListItemText sx={{ textTransform: "capitalize" }} primary={props.label ? props.label : "label"}></ListItemText>

                </ListItem>
            </ListItemButton>
        </Paper>
    )

}