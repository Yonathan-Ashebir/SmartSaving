import { ArrowBack, ArrowBackIos, ArrowRightAlt } from "@mui/icons-material";
import { Button, Paper, useTheme, withStyles, withTheme, List, Stack, Container, ListItem } from "@mui/material";
import React, { useState } from "react"
import { getGroupsData } from "../logic/data.ts"
import { AddButton } from "./addButton.tsx";
import DataEntry from "./dataEntry.tsx";
import { DeviceGroup } from "./deviceGroup.tsx"
import { DeviceItem } from "./deviceItem.tsx";


const HOME_STATES = { LIST: 0, GROUP: 1, DEVICE: 2 };
function Home(props) {
    let [state, setState] = useState(HOME_STATES.LIST)
    let [groupName, setGroupName] = useState("Group name");
    let [deviceName, setDeviceName] = useState("Device name")
    let theme = useTheme()

    let groups = []
    let groupsData = getGroupsData();
    let getDevices = (): [] => {
        if (groupName != "Group name") return groupsData[groupName]; else return []
    }
    let goBack = () => { if (state > 0) setState(state - 1) }
    for (const groupName in groupsData) {
        groups.push({ name: groupName, count: groupsData[groupName].length })
    }
    return (

        <Stack direction="column" style={{ position: "relative", flexGrow: 100, background: "transparent", justifyContent: "stretch" }}>
            <Stack direction="row">
                {(state > 0) ? <Button sx={{ minWidth: 0, margin: 0, padding: 0, paddingLeft: "3mm" }} onClick={goBack}>< ArrowBackIos /></Button> : null}
                <Container style={{ padding: 0, margin: 0 }}>
                    <h2 style={{ textTransform: "capitalize" }}>{(state === HOME_STATES.LIST) ? "Rooms" : (state === HOME_STATES.GROUP) ? groupName + " > Devices" : groupName + " > " + deviceName}</h2>
                </Container></Stack>
            <List style={{ flexGrow: "3", borderRadius: "5mm 5mm 0mm 0mm", backgroundColor: "#ffffff77" }}>
                {(state === HOME_STATES.LIST) ?
                    groups.map((group) => {
                        return (
                            <DeviceGroup openGroup={() => { setGroupName(group.name); setState(HOME_STATES.GROUP); }} label={group.name} count={group.count}>
                            </DeviceGroup>
                        )
                    })
                    : (state === HOME_STATES.GROUP) ?
                        getDevices().map((device) =>
                            <DeviceItem openDevice={() => { setDeviceName(device.name); setState(HOME_STATES.DEVICE); }} data={device} label={device.name}></DeviceItem>
                        ) : <Stack>
                            {getDevices().filter((device) => device.name == deviceName).map((device) =>/* todo: to improve */
                                <Stack divider={<hr></hr>}>
                                    <Paper elevation={3} style={{ borderRadius: "5mm", margin: "5mm 3mm", overflow: "hidden" }}> <DataEntry label="Name:" type="text-edit" data={deviceName}></DataEntry>
                                        <DataEntry label="State:" type="switch" data={device.state}></DataEntry></Paper>

                                    <Paper elevation={3} style={{ borderRadius: "5mm", margin: "5mm 3mm", overflow: "hidden" }}> <DataEntry label="Interface:" type="display" data={device.interface}></DataEntry></Paper>
                                    <Paper elevation={3} style={{ borderRadius: "5mm", margin: "5mm 3mm", overflow: "hidden" }}>  <DataEntry label="Turn off Delay:" type="number-edit" data={30}></DataEntry>
                                        <DataEntry label="Auto turn on:" type="display" data={"3:00pm"}></DataEntry>
                                        <DataEntry label="Auto turn off" type="display" data="4:00 pm"></DataEntry></Paper>

                                </Stack>
                            )}
                        </Stack>

                }
                {(state === HOME_STATES.LIST) ? < AddButton onClick={() => { }} itemName="group"></AddButton> : (state === HOME_STATES.GROUP) ? < AddButton onClick={() => { }} itemName="device"></AddButton> : null}
            </List >

        </Stack >
    )
}

export default Home