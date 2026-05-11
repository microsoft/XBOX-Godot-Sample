// GENERATED FILE - DO NOT EDIT BY HAND.
#ifndef GODOT_PLAYFAB_GENERATED_SERVICES_H
#define GODOT_PLAYFAB_GENERATED_SERVICES_H
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/variant.hpp>
#include "playfab_generated_api_helpers.h"

namespace godot {
class PlayFab; class PlayFabRuntime; class PlayFabUser;

class PlayFabAccounts : public RefCounted {
    GDCLASS(PlayFabAccounts, RefCounted);
    PlayFab *m_owner = nullptr;
    PlayFabRuntime *_get_runtime() const;
protected:
    static void _bind_methods();
public:
    void set_owner(PlayFab *p_owner);
    Signal add_or_update_contact_email_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal get_account_info_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal get_player_combined_info_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal get_player_profile_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal get_play_fab_ids_from_battle_net_account_ids_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal get_play_fab_ids_from_google_ids_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal get_play_fab_ids_from_kongregate_ids_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal get_play_fab_ids_from_steam_ids_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal get_play_fab_ids_from_steam_names_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal get_play_fab_ids_from_xbox_live_ids_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal link_battle_net_account_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal link_custom_id_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal link_open_id_connect_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal link_steam_account_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal link_xbox_account_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal remove_contact_email_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal report_player_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal unlink_battle_net_account_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal unlink_custom_id_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal unlink_open_id_connect_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal unlink_steam_account_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal unlink_xbox_account_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal update_avatar_url_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal update_user_title_display_name_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal get_title_players_from_xbox_live_ids_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal set_display_name_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal get_profile_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal get_profiles_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal get_title_players_from_master_player_account_ids_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal set_profile_language_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal set_profile_policy_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
};

class PlayFabCatalog : public RefCounted {
    GDCLASS(PlayFabCatalog, RefCounted);
    PlayFab *m_owner = nullptr;
    PlayFabRuntime *_get_runtime() const;
protected:
    static void _bind_methods();
public:
    void set_owner(PlayFab *p_owner);
    Signal create_draft_item_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal create_upload_urls_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal delete_entity_item_reviews_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal delete_item_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal get_catalog_config_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal get_draft_item_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal get_draft_items_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal get_entity_draft_items_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal get_entity_item_review_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal get_item_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal get_item_containers_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal get_item_moderation_state_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal get_item_publish_status_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal get_item_reviews_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal get_item_review_summary_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal get_items_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal publish_draft_item_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal report_item_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal report_item_review_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal review_item_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal search_items_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal set_item_moderation_state_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal submit_item_review_vote_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal takedown_item_reviews_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal update_catalog_config_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal update_draft_item_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
};

class PlayFabCloudScript : public RefCounted {
    GDCLASS(PlayFabCloudScript, RefCounted);
    PlayFab *m_owner = nullptr;
    PlayFabRuntime *_get_runtime() const;
protected:
    static void _bind_methods();
public:
    void set_owner(PlayFab *p_owner);
    Signal execute_cloud_script_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal execute_entity_cloud_script_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal execute_function_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
};

class PlayFabEntityData : public RefCounted {
    GDCLASS(PlayFabEntityData, RefCounted);
    PlayFab *m_owner = nullptr;
    PlayFabRuntime *_get_runtime() const;
protected:
    static void _bind_methods();
public:
    void set_owner(PlayFab *p_owner);
    Signal abort_file_uploads_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal delete_files_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal finalize_file_uploads_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal get_files_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal get_objects_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal initiate_file_uploads_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal set_objects_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
};

class PlayFabEvents : public RefCounted {
    GDCLASS(PlayFabEvents, RefCounted);
    PlayFab *m_owner = nullptr;
    PlayFabRuntime *_get_runtime() const;
protected:
    static void _bind_methods();
public:
    void set_owner(PlayFab *p_owner);
};

class PlayFabExperimentation : public RefCounted {
    GDCLASS(PlayFabExperimentation, RefCounted);
    PlayFab *m_owner = nullptr;
    PlayFabRuntime *_get_runtime() const;
protected:
    static void _bind_methods();
public:
    void set_owner(PlayFab *p_owner);
    Signal get_treatment_assignment_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
};

class PlayFabFriends : public RefCounted {
    GDCLASS(PlayFabFriends, RefCounted);
    PlayFab *m_owner = nullptr;
    PlayFabRuntime *_get_runtime() const;
protected:
    static void _bind_methods();
public:
    void set_owner(PlayFab *p_owner);
    Signal add_friend_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal get_friends_list_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal remove_friend_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal set_friend_tags_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
};

class PlayFabGroups : public RefCounted {
    GDCLASS(PlayFabGroups, RefCounted);
    PlayFab *m_owner = nullptr;
    PlayFabRuntime *_get_runtime() const;
protected:
    static void _bind_methods();
public:
    void set_owner(PlayFab *p_owner);
    Signal accept_group_application_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal accept_group_invitation_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal add_members_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal apply_to_group_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal block_entity_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal change_member_role_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal create_group_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal create_role_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal delete_group_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal delete_role_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal get_group_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal invite_to_group_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal is_member_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal list_group_applications_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal list_group_blocks_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal list_group_invitations_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal list_group_members_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal list_membership_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal list_membership_opportunities_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal remove_group_application_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal remove_group_invitation_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal remove_members_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal unblock_entity_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal update_group_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal update_role_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
};

class PlayFabInventory : public RefCounted {
    GDCLASS(PlayFabInventory, RefCounted);
    PlayFab *m_owner = nullptr;
    PlayFabRuntime *_get_runtime() const;
protected:
    static void _bind_methods();
public:
    void set_owner(PlayFab *p_owner);
    Signal add_inventory_items_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal delete_inventory_collection_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal delete_inventory_items_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal execute_inventory_operations_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal execute_transfer_operations_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal get_inventory_collection_ids_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal get_inventory_items_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal get_inventory_operation_status_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal get_transaction_history_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal purchase_inventory_items_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal redeem_google_play_inventory_items_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal redeem_microsoft_store_inventory_items_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal redeem_play_station_store_inventory_items_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal redeem_steam_inventory_items_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal subtract_inventory_items_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal transfer_inventory_items_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal update_inventory_items_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
};

class PlayFabLocalization : public RefCounted {
    GDCLASS(PlayFabLocalization, RefCounted);
    PlayFab *m_owner = nullptr;
    PlayFabRuntime *_get_runtime() const;
protected:
    static void _bind_methods();
public:
    void set_owner(PlayFab *p_owner);
    Signal get_language_list_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
};

class PlayFabPlayerData : public RefCounted {
    GDCLASS(PlayFabPlayerData, RefCounted);
    PlayFab *m_owner = nullptr;
    PlayFabRuntime *_get_runtime() const;
protected:
    static void _bind_methods();
public:
    void set_owner(PlayFab *p_owner);
    Signal delete_player_custom_properties_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal get_player_custom_property_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal get_user_data_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal get_user_publisher_data_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal get_user_publisher_read_only_data_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal get_user_read_only_data_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal list_player_custom_properties_async(const Ref<PlayFabUser> &p_user);
    Signal update_player_custom_properties_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal update_user_data_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal update_user_publisher_data_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
};

class PlayFabStatistics : public RefCounted {
    GDCLASS(PlayFabStatistics, RefCounted);
    PlayFab *m_owner = nullptr;
    PlayFabRuntime *_get_runtime() const;
protected:
    static void _bind_methods();
public:
    void set_owner(PlayFab *p_owner);
    Signal create_statistic_definition_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal delete_statistic_definition_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal delete_statistics_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal get_statistic_definition_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal get_statistics_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal get_statistics_for_entities_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal increment_statistic_version_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal list_statistic_definitions_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal update_statistic_definition_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal update_statistics_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
};

class PlayFabTitleData : public RefCounted {
    GDCLASS(PlayFabTitleData, RefCounted);
    PlayFab *m_owner = nullptr;
    PlayFabRuntime *_get_runtime() const;
protected:
    static void _bind_methods();
public:
    void set_owner(PlayFab *p_owner);
    Signal get_publisher_data_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal get_time_async(const Ref<PlayFabUser> &p_user);
    Signal get_title_data_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
    Signal get_title_news_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request = Dictionary());
};

}
#endif
