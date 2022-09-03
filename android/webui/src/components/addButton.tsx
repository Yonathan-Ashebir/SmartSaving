import { Add } from "@mui/icons-material";
import { Button, Container, Stack } from "@mui/material";
import React from "react"
export function AddButton(props) {
    return (
        <Container>
            <Button onClick={props.onClick}sx={{ borderRadius: "3mm" }}><Add />{" Add "+props.itemName}</Button>
        </Container>
    )
}