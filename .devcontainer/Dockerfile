FROM redhat/ubi9:latest

# 安装开发工具（包含 GCC）
RUN dnf install -y \
    git \
    curl \
    wget \
    sudo \
    vim \
    gcc \
    gcc-c++ \
    gdb \
    make \
    && dnf clean all

# 设置非 root 用户
ARG USERNAME=vscode
ARG USER_UID=1000
ARG USER_GID=$USER_UID

RUN groupadd --gid $USER_GID $USERNAME \
    && useradd --uid $USER_UID --gid $USER_GID -m $USERNAME \
    && echo $USERNAME ALL=\(root\) NOPASSWD:ALL > /etc/sudoers.d/$USERNAME \
    && chmod 0440 /etc/sudoers.d/$USERNAME

USER $USERNAME

# 验证 GCC 安装（可选）
RUN gcc --version && g++ --version
