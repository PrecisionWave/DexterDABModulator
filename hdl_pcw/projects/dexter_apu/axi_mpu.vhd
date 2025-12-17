library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

use work.empty_axi_pkg.all;

entity axi_mpu is
    port (
        aclk              : in  std_logic;
        arstn             : in  std_logic;

        M_AXI_awid        : out std_logic_vector(1 downto 0);
        M_AXI_awaddr      : out std_logic_vector(31 downto 0);
        M_AXI_awlen       : out std_logic_vector(3 downto 0);
        M_AXI_awsize      : out std_logic_vector(2 downto 0);
        M_AXI_awburst     : out std_logic_vector(1 downto 0);
        M_AXI_awlock      : out std_logic_vector(1 downto 0);
        M_AXI_awcache     : out std_logic_vector(3 downto 0);
        M_AXI_awprot      : out std_logic_vector(2 downto 0);
        M_AXI_awvalid     : out std_logic;
        M_AXI_awready     : in  std_logic;

        M_AXI_wid         : out std_logic_vector(1 downto 0);
        M_AXI_wdata       : out std_logic_vector(63 downto 0);
        M_AXI_wstrb       : out std_logic_vector(7 downto 0);
        M_AXI_wlast       : out std_logic;
        M_AXI_wvalid      : out std_logic;
        M_AXI_wready      : in  std_logic;

        M_AXI_bid         : in  std_logic_vector(1 downto 0);
        M_AXI_bresp       : in  std_logic_vector(1 downto 0);
        M_AXI_bvalid      : in  std_logic;
        M_AXI_bready      : out std_logic;

        M_AXI_arid        : out std_logic_vector(1 downto 0);
        M_AXI_araddr      : out std_logic_vector(31 downto 0);
        M_AXI_arlen       : out std_logic_vector(3 downto 0);
        M_AXI_arsize      : out std_logic_vector(2 downto 0);
        M_AXI_arburst     : out std_logic_vector(1 downto 0);
        M_AXI_arlock      : out std_logic_vector(1 downto 0);
        M_AXI_arcache     : out std_logic_vector(3 downto 0);
        M_AXI_arprot      : out std_logic_vector(2 downto 0);
        M_AXI_arvalid     : out std_logic;
        M_AXI_arready     : in  std_logic;

        M_AXI_rid         : in  std_logic_vector(1 downto 0);
        M_AXI_rdata       : in  std_logic_vector(63 downto 0);
        M_AXI_rresp       : in  std_logic_vector(1 downto 0);
        M_AXI_rlast       : in  std_logic;
        M_AXI_rvalid      : in  std_logic;
        M_AXI_rready      : out std_logic;

        S_AXI_awid        : in  std_logic_vector(1 downto 0);
        S_AXI_awaddr      : in  std_logic_vector(31 downto 0);
        S_AXI_awlen       : in  std_logic_vector(3 downto 0);
        S_AXI_awsize      : in  std_logic_vector(2 downto 0);
        S_AXI_awburst     : in  std_logic_vector(1 downto 0);
        S_AXI_awlock      : in  std_logic_vector(1 downto 0);
        S_AXI_awcache     : in  std_logic_vector(3 downto 0);
        S_AXI_awprot      : in  std_logic_vector(2 downto 0);
        S_AXI_awvalid     : in  std_logic;
        S_AXI_awready     : out std_logic;

        S_AXI_wid         : in  std_logic_vector(1 downto 0);
        S_AXI_wdata       : in  std_logic_vector(63 downto 0);
        S_AXI_wstrb       : in  std_logic_vector(7 downto 0);
        S_AXI_wlast       : in  std_logic;
        S_AXI_wvalid      : in  std_logic;
        S_AXI_wready      : out std_logic;

        S_AXI_bid         : out std_logic_vector(1 downto 0);
        S_AXI_bresp       : out std_logic_vector(1 downto 0);
        S_AXI_bvalid      : out std_logic;
        S_AXI_bready      : in  std_logic;

        S_AXI_arid        : in  std_logic_vector(1 downto 0);
        S_AXI_araddr      : in  std_logic_vector(31 downto 0);
        S_AXI_arlen       : in  std_logic_vector(3 downto 0);
        S_AXI_arsize      : in  std_logic_vector(2 downto 0);
        S_AXI_arburst     : in  std_logic_vector(1 downto 0);
        S_AXI_arlock      : in  std_logic_vector(1 downto 0);
        S_AXI_arcache     : in  std_logic_vector(3 downto 0);
        S_AXI_arprot      : in  std_logic_vector(2 downto 0);
        S_AXI_arvalid     : in  std_logic;
        S_AXI_arready     : out std_logic;

        S_AXI_rid         : out std_logic_vector(1 downto 0);
        S_AXI_rdata       : out std_logic_vector(63 downto 0);
        S_AXI_rresp       : out std_logic_vector(1 downto 0);
        S_AXI_rlast       : out std_logic;
        S_AXI_rvalid      : out std_logic;
        S_AXI_rready      : in  std_logic;

        s_axi_ctl_araddr  : in  std_logic_vector(11 downto 0);
        s_axi_ctl_arready : out std_logic;
        s_axi_ctl_arvalid : in  std_logic;

        s_axi_ctl_awaddr  : in  std_logic_vector(11 downto 0);
        s_axi_ctl_awready : out std_logic;
        s_axi_ctl_awvalid : in  std_logic;

        s_axi_ctl_bready  : in  std_logic;
        s_axi_ctl_bresp   : out std_logic_vector(1 downto 0);
        s_axi_ctl_bvalid  : out std_logic;

        s_axi_ctl_rdata   : out std_logic_vector(31 downto 0);
        s_axi_ctl_rready  : in  std_logic;
        s_axi_ctl_rresp   : out std_logic_vector(1 downto 0);
        s_axi_ctl_rvalid  : out std_logic;

        s_axi_ctl_wdata   : in  std_logic_vector(31 downto 0);
        s_axi_ctl_wready  : out std_logic;
        s_axi_ctl_wstrb   : in  std_logic_vector(3 downto 0);
        s_axi_ctl_wvalid  : in  std_logic
    );
end entity;

architecture rtl of axi_mpu is
begin

    M_AXI_awid    <= S_AXI_awid;
    M_AXI_awaddr  <= S_AXI_awaddr;
    M_AXI_awlen   <= S_AXI_awlen;
    M_AXI_awsize  <= S_AXI_awsize;
    M_AXI_awburst <= S_AXI_awburst;
    M_AXI_awlock  <= S_AXI_awlock;
    M_AXI_awcache <= S_AXI_awcache;
    M_AXI_awprot  <= S_AXI_awprot;
    M_AXI_awvalid <= S_AXI_awvalid;
    S_AXI_awready <= M_AXI_awready;

    M_AXI_wid     <= S_AXI_wid;
    M_AXI_wdata   <= S_AXI_wdata;
    M_AXI_wstrb   <= S_AXI_wstrb;
    M_AXI_wlast   <= S_AXI_wlast;
    M_AXI_wvalid  <= S_AXI_wvalid;
    S_AXI_wready  <= M_AXI_wready;

    S_AXI_bid     <= M_AXI_bid;
    S_AXI_bresp   <= M_AXI_bresp;
    S_AXI_bvalid  <= M_AXI_bvalid;
    M_AXI_bready  <= S_AXI_bready;

    M_AXI_arid    <= S_AXI_arid;
    M_AXI_araddr  <= S_AXI_araddr;
    M_AXI_arlen   <= S_AXI_arlen;
    M_AXI_arsize  <= S_AXI_arsize;
    M_AXI_arburst <= S_AXI_arburst;
    M_AXI_arlock  <= S_AXI_arlock;
    M_AXI_arcache <= S_AXI_arcache;
    M_AXI_arprot  <= S_AXI_arprot;
    M_AXI_arvalid <= S_AXI_arvalid;
    S_AXI_arready <= M_AXI_arready;

    S_AXI_rid     <= M_AXI_rid;
    S_AXI_rdata   <= M_AXI_rdata;
    S_AXI_rresp   <= M_AXI_rresp;
    S_AXI_rlast   <= M_AXI_rlast;
    S_AXI_rvalid  <= M_AXI_rvalid;
    M_AXI_rready  <= S_AXI_rready;

    csr_i : entity work.empty_axi
    generic map(
        READ_RESPONSE  => OKAY,
        WRITE_RESPONSE => OKAY
    )
    port map(
        S_AXI_ACLK    => aclk,
        S_AXI_ARESETN => arstn,

        S_AXI_AWVALID => s_axi_ctl_awvalid,
        S_AXI_AWREADY => s_axi_ctl_awready,

        S_AXI_WVALID  => s_axi_ctl_wvalid,
        S_AXI_WREADY  => s_axi_ctl_wready,

        S_AXI_BVALID  => s_axi_ctl_bvalid,
        S_AXI_BREADY  => s_axi_ctl_bready,
        S_AXI_BRESP   => s_axi_ctl_bresp,

        S_AXI_ARVALID => s_axi_ctl_arvalid,
        S_AXI_ARREADY => s_axi_ctl_arready,

        S_AXI_RVALID  => s_axi_ctl_rvalid,
        S_AXI_RREADY  => s_axi_ctl_rready,
        S_AXI_RRES    => s_axi_ctl_rresp
    );

    s_axi_ctl_rdata <= x"DEADF00D";
end architecture;