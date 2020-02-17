# Smart curtains
IOT project to connect curtains and open/close them automatically using the REST API. This code is meant to be used on a NodeMCU ESP8266 WiFi board.

The 3D designed pulley of this project (with the rope in blue) is depicted below:
![Pulley with rope](https://github.com/Nicwalle/smart-curtains-arduino/blob/master/pulley.png)

## REST API

| Method | URI | Params | Description |
|:------:|:---:|:------:|:-----------:|
| GET | /get-state | - | Gets the state of the curtains (state, objective, openState, closedState) |
| POST | /set-state | state : int | Sets the *state* and *objective* to the **[state]** value |
| POST | /reset | (open : int) <br> (closed : int) | Sets the *state* and *objective* value to **0** and, if set, sets the *openState* to **[open]** and *closedState* to **[closed]** |
| POST | /set-bounds | (open : int) <br> (closed : int) | If set, sets the *openState* to **[open]** and *closedState* to **[closed]** |
| POST | /open | - | Sets the *objective* value to *openState* |
| POST | /close | - | Sets the *objective* value to *closedState* |
| POST | /goto | goal : int | Sets the *objective* value to **[goal]** |
| POST | /stop | - | Sets the *objective* to the current *state*|
