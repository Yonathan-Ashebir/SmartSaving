import { useTheme } from "@mui/material/styles"
import { ListItem, ListItemButton, ListItemIcon, ListItemText, Paper } from "@mui/material";
import { width } from "@mui/system";
import React from "react"
import { Bathroom, Bed, DeviceHub, Fence, Kitchen, Living, Room, RoomPreferences } from "@mui/icons-material";
export function DeviceGroup(props) {
    let theme = useTheme();
    let icon;
    switch(props.label){
        case "living room": {icon = <Living/>;break}
        case "bed room": {icon = <Bed/>;break}
        case "bath room": {icon = <Bathroom/>;break}
        case "kitchen": {icon = <Kitchen/>;break}
        case "compound":{icon= <Fence/>;break}
        default: icon = <RoomPreferences/>;
    }
    return (
        
        <Paper elevation={3} style={{ borderRadius: "5mm", margin: "5mm 3mm" ,overflow:"hidden"}}>
            <ListItemButton onClick={props.openGroup}>
                <ListItem
                    secondaryAction={<span style={{ color: theme.palette.text.disabled }}>{props.count ? props.count + " devices" : "no devices"}</span>}
                >

                    <ListItemIcon sx={{ minWidth: "9mm" }}>{icon}</ListItemIcon>
                    <ListItemText sx={{textTransform:"capitalize"}}primary={props.label ? props.label : "label"}></ListItemText>

                </ListItem>
                  </ListItemButton>
        </Paper>
    )

}