import React from 'react';
import { sendCommand, connectChangerCeiling, endConnectionCeiling, changeColorCeiling, connectChangerCouch, endConnectionCouch, changeColorCouch, lightOptions } from './services/additionalFunctions'
import { ChromePicker } from 'react-color';
import Select from 'react-select';
import Button from 'react-bootstrap/Button';
import InputNumber from 'rc-input-number';

class App extends React.Component {
  state = {
    background: '#fff',
    selectedOption: { value: "1colceiling" },
    isConnectedCeiling: false,
    isConnectedCouch: false,
    fadeSpeed: 50,
  }

  // Runs everytime a color changes
  handleChange = async (color, event) => {
    this.setState({ background: color.hex });
    // let option = this.state.selectedOption.value;
    // let newColStr = ""
    //
    // // Create string to send data
    // if(option === "1colceiling" || option === "1colcouch")
    //   newColStr = color.rgb.r + " " + color.rgb.g + " " + color.rgb.b + " 1";
    // else if(option === "2colceiling" || option === "2colcouch")
    //   newColStr = color.rgb.r + " " + color.rgb.g + " " + color.rgb.b + " 2";
    // else
    //   newColStr = color.rgb.r + " " + color.rgb.g + " " + color.rgb.b + " 0";
    //
    // console.log(newColStr);
    //
    // if(option.includes("ceiling") || option === "all") {
    //   if(!this.state.isConnectedCeiling) {
    //     console.log("Starting ceiling connection")
    //     await connectChangerCeiling();
    //     console.log("Ceiling connected");
    //     this.setState({ isConnectedCeiling: true });
    //   }
    //
    //   if(newColStr.length > 1) {
    //     console.log('Sending new color')
    //     changeColorCeiling(newColStr);
    //     console.log('Sent new color')
    //   }
    // }
    //
    // if(option.includes("couch") || option === "all") {
    //   if(!this.state.isConnectedCouch) {
    //     console.log("Starting couch connection")
    //     await connectChangerCouch();
    //     console.log("Couch connected");
    //     this.setState({ isConnectedCouch: true });
    //   }
    //
    //   if(newColStr.length > 1) {
    //     console.log('Sending new color')
    //     changeColorCouch(newColStr);
    //     console.log('Sent new color')
    //   }
    // }
  }

  // Kill connection when colors aren't being changed
  handleChangeComplete = async (color, event) => {
    let option = this.state.selectedOption.value;
    let newColStr = ""

    // sample line: 255-0-0-0-0- > sets red to high others to low on both strips
    if(option === "1colceiling" || option === "1colcouch")
      newColStr = color.rgb.r + "-" + color.rgb.g + "-" + color.rgb.b + "-1-0-";
    else if(option === "2colceiling" || option === "2colcouch")
      newColStr = color.rgb.r + "-" + color.rgb.g + "-" + color.rgb.b + "-2-0-";
    else
      newColStr = color.rgb.r + "-" + color.rgb.g + "-" + color.rgb.b + "-0-0-";

    await sendCommand(newColStr);

    // if(option.includes("ceiling")) {
    //   if(!this.state.isConnectedCeiling) {
    //     console.log("Starting ceiling connection")
    //     await connectChangerCeiling();
    //     console.log("Ceiling connected");
    //     this.setState({ isConnectedCeiling: true });
    //   }
    //
    //   if(this.state.isConnectedCeiling) {
    //     // Ensure color is set - just send same command 5 times
    //     // Bad way to do it - need to figure out better method later
    //     for(let i = 0; i < 5; i++)
    //       await changeColorCeiling(newColStr);
    //
    //     endConnectionCeiling();
    //     this.setState({ isConnectedCeiling: false });
    //   }
    // } else if(option.includes("couch")) {
    //   if(!this.state.isConnectedCouch) {
    //     console.log("Starting couch connection")
    //     await connectChangerCouch();
    //     console.log("Couch connected");
    //     this.setState({ isConnectedCouch: true });
    //   }
    //
    //   if(this.state.isConnectedCouch) {
    //     // Ensure color is set - just send same command 5 times
    //     // Bad way to do it - need to figure out better method later
    //     for(let i = 0; i < 5; i++)
    //       await changeColorCouch(newColStr);
    //
    //     endConnectionCouch();
    //     this.setState({ isConnectedCouch: false });
    //   }
    // } else if(option === "all") {
    //   if(!this.state.isConnectedCeiling) {
    //     console.log("Starting ceiling connection")
    //     await connectChangerCeiling();
    //     console.log("Ceiling connected");
    //     this.setState({ isConnectedCeiling: true });
    //   }
    //
    //   if(!this.state.isConnectedCouch) {
    //     console.log("Starting couch connection")
    //     await connectChangerCouch();
    //     console.log("Couch connected");
    //     this.setState({ isConnectedCouch: true });
    //   }
    //
    //   if(this.state.isConnectedCouch && this.state.isConnectedCeiling) {
    //     for(let i = 0; i < 5; i++) {
    //       await changeColorCeiling(newColStr);
    //       await changeColorCouch(newColStr);
    //     }
    //
    //     endConnectionCouch();
    //     this.setState({ isConnectedCouch: false });
    //
    //     endConnectionCeiling();
    //     this.setState({ isConnectedCeiling: false });
    //   }
    //
    // }
  }

  // Handle strip selection dropdown menu
  handleSelectChange = selectedOption => {
    this.setState(
      { selectedOption },
      () => console.log(`Option selected:`, this.state.selectedOption)
    );
  };

  onNumChange = fadeSpeed => {
    this.setState({fadeSpeed});
  }

  render() {
    return (
      <div>
        <center>
          <ChromePicker
            color = { this.state.background }
            onChangeComplete = { this.handleChangeComplete }
            onChange = { this.handleChange }
          />

          <br/>

          <Select
              className = "basic-single"
              classNamePrefix = "select"
              defaultValue = {lightOptions[0]}
              isDisabled = {false}
              isLoading = {false}
              isClearable = {false}
              isRtl = {false}
              isSearchable = {false}
              name = "color"
              options = {lightOptions}
              onChange = {this.handleSelectChange}
          />

          <br/>

          <InputNumber min = {10}
                       value = {this.state.fadeSpeed}
                       defaultValue = {50}
                       step = {1}
                       style = {{margin: 10}}
                       onChange = {this.onNumChange}
          />

         <Button variant="outline-dark" onClick={async () => {
              if(!this.state.isConnectedCeiling) {
                console.log("Starting ceiling connection")
                await connectChangerCeiling();
                console.log("Ceiling connected");
                this.setState({ isConnectedCeiling: true });
              }

              if(!this.state.isConnectedCouch) {
                console.log("Starting couch connection")
                await connectChangerCouch();
                console.log("Couch connected");
                this.setState({ isConnectedCouch: true });
              }

              if(this.state.isConnectedCouch && this.state.isConnectedCeiling) {
                let newColStr = "0-0-0-3-" + this.state.fadeSpeed + "-";
                for(let i = 0; i < 5; i++) {
                  await changeColorCeiling(newColStr);
                  await changeColorCouch(newColStr);
                }

                await endConnectionCouch();
                this.setState({ isConnectedCouch: false });

                await endConnectionCeiling();
                this.setState({ isConnectedCeiling: false });
              }
          }}>Start Fade (delay above)</Button>
        </center>
      </div>
    );
  }
}

export default App;
