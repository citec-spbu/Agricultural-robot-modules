Step by step:

`mkdir agro`\
`cd agro`

`git clone https://github.com/citec-spbu/Agricultural-robot-modules.git AgroDesktop`\
`cd AgroDesktop`\
`git checkout AgroDesktop`

`git worktree add ../ml_service ml_service`\
`git worktree add ../robotsim_service robot_sim_testing`

Download file https://download.pytorch.org/models/mobilenet_v3_large-8738ca79.pth and put it in `agro/ml_service/weights/torch/hub/checkpoints/`

`docker compose up --build`

\
\
\
Result:\
ML service port: http://localhost:8001 \
RobotSim service port: http://localhost:8002