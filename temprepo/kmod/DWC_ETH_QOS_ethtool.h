#ifndef __DWC_ETH_QOS_ETHTOOL_H__

#define __DWC_ETH_QOS_ETHTOOL_H__

static void DWC_ETH_QOS_get_pauseparam(struct net_device *dev,
				       struct ethtool_pauseparam *pause);
static int DWC_ETH_QOS_set_pauseparam(struct net_device *dev,
				      struct ethtool_pauseparam *pause);

static int DWC_ETH_QOS_getsettings(struct net_device *dev,
				   struct ethtool_cmd *cmd);
static int DWC_ETH_QOS_setsettings(struct net_device *dev,
				   struct ethtool_cmd *cmd);
#if 0
static void DWC_ETH_QOS_get_wol(struct net_device *dev,
				struct ethtool_wolinfo *wol);
static int DWC_ETH_QOS_set_wol(struct net_device *dev,
			       struct ethtool_wolinfo *wol);
#endif
static int DWC_ETH_QOS_set_coalesce(struct net_device *dev,
				    struct ethtool_coalesce *ec);
static int DWC_ETH_QOS_get_coalesce(struct net_device *dev,
				    struct ethtool_coalesce *ec);

static int DWC_ETH_QOS_get_sset_count(struct net_device *dev, int sset);

static void DWC_ETH_QOS_get_strings(struct net_device *dev, u32 stringset, u8 *data);

static void DWC_ETH_QOS_get_ethtool_stats(struct net_device *dev,
	struct ethtool_stats *dummy, u64 *data);

static int DWC_ETH_QOS_get_ts_info(struct net_device *dev,
		struct ethtool_ts_info *info);

//static int DWC_ETH_QOS_set_tso(struct net_device *dev, u32 data);
//static u32 DWC_ETH_QOS_get_tso(struct net_device *dev);

#endif
