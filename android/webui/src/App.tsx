import logo from './logo.svg';
import './App.css';
import Home from './components/home.tsx';
import { DeviceHub, List, Settings } from '@mui/icons-material';
import { BottomNavigation, BottomNavigationAction,Stack } from '@mui/material';
import React,{ useState } from 'react';

function App() {
  let [page, setPage] = useState(0)
  console.log("page:",page)
  return (
    <Stack className="App" sx={{background:"linear-gradient(104deg, #0000ff22 50% , transparent)",minHeight:"100vh",
    justifyContent: "stretch"
}}>
      {(page==0)?<Home></Home>:<div style={{flexGrow:100,flexBasis:"5cm",height:"5cm"}}></div>}

      <BottomNavigation style={{justifySelf:"flex-end"}}
        value={page}
        onChange={(event, newValue) => {
          setPage(newValue);
        }}
      >
        <BottomNavigationAction label="Devices" icon={<DeviceHub />} />
        <BottomNavigationAction label="Settings" icon={<Settings />} />
      </BottomNavigation>
    </Stack>
  );
}

export default App;
