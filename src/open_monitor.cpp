extern "C" {
#include <locale.h>
#include <glib.h>
#include <gst/gst.h>
#include <gst/sdp/sdp.h>

#ifdef G_OS_UNIX
#include <glib-unix.h>
#endif

#define GST_USE_UNSTABLE_API
#include <gst/webrtc/webrtc.h>

#include <libsoup/soup.h>
#include <json-glib/json-glib.h>
}

#include <memory>
#include <iostream>

#include <glibmm/convert.h>
#include <glibmm/init.h>
#include <glibmm/main.h>
#include <glibmm/optioncontext.h>

#include "index.html.h"

using namespace std;

constexpr auto soupHttpPort = 57778U;

shared_ptr<GstElement> pipeline;

template<typename T>
inline auto make_unique_gobject(T* object) noexcept {
    g_assert_nonnull(object);

    return unique_ptr<T, function<void(T*)>>(
        object,
        [](auto ptr) {
            gst_object_unref(ptr);
        }
    );
}

template<typename T>
using unique_gobject_ptr = unique_ptr<T, function<void(T*)>>;

// Everything in this extern "C" scope is untouched code from the webrtc_unidirectional_h264 example
extern "C" {
#define RTP_AUDIO_PAYLOAD_TYPE "97"

#ifdef G_OS_WIN32
#define VIDEO_SRC "mfvideosrc"
#else
#define VIDEO_SRC "v4l2src"
#endif

    gchar *video_priority = NULL;
    gchar *audio_priority = NULL;


    typedef struct _ReceiverEntry ReceiverEntry;

    ReceiverEntry *create_receiver_entry (SoupWebsocketConnection * connection, GstElement* pipeline);

    void on_offer_created_cb (GstPromise * promise, gpointer user_data);
    void on_negotiation_needed_cb (GstElement * webrtcbin, gpointer user_data);
    void on_ice_candidate_cb (GstElement * webrtcbin, guint mline_index,
        gchar * candidate, gpointer user_data);

    void soup_websocket_message_cb (SoupWebsocketConnection * connection,
        SoupWebsocketDataType data_type, GBytes * message, gpointer user_data);
    void soup_websocket_closed_cb (SoupWebsocketConnection * connection,
        gpointer user_data);

    void soup_http_handler (SoupServer * soup_server, SoupMessage * message,
        const char *path, GHashTable * query, SoupClientContext * client_context,
        gpointer user_data);

    static gchar *get_string_from_json_object (JsonObject * object);

    struct _ReceiverEntry
    {
      SoupWebsocketConnection *connection;

      GstElement *webrtcbin;
    };

    static gboolean
    bus_watch_cb ([[maybe_unused]] GstBus * bus, GstMessage * message, [[maybe_unused]] gpointer user_data)
    {
      switch (GST_MESSAGE_TYPE (message)) {
        case GST_MESSAGE_ERROR:
        {
          GError *error = NULL;
          gchar *debug = NULL;

          gst_message_parse_error (message, &error, &debug);
          g_error ("Error on bus: %s (debug: %s)", error->message, debug);
          g_error_free (error);
          g_free (debug);
          break;
        }
        case GST_MESSAGE_WARNING:
        {
          GError *error = NULL;
          gchar *debug = NULL;

          gst_message_parse_warning (message, &error, &debug);
          g_warning ("Warning on bus: %s (debug: %s)", error->message, debug);
          g_error_free (error);
          g_free (debug);
          break;
        }
        default:
          break;
      }

      return G_SOURCE_CONTINUE;
    }

    //static GstWebRTCPriorityType
    //_priority_from_string (const gchar * s)
    //{
      //GEnumClass *klass =
          //(GEnumClass *) g_type_class_ref (GST_TYPE_WEBRTC_PRIORITY_TYPE);
      //GEnumValue *en;

      //g_return_val_if_fail (klass, GST_WEBRTC_PRIORITY_TYPE_VERY_LOW);
      //if (!(en = g_enum_get_value_by_name (klass, s)))
        //en = g_enum_get_value_by_nick (klass, s);
      //g_type_class_unref (klass);

      //if (en) {
          //switch(en->value) {
            //case 0: [[fallthrough]];
            //case 1:
                //return GST_WEBRTC_PRIORITY_TYPE_VERY_LOW;
            //case 2:
                //return GST_WEBRTC_PRIORITY_TYPE_LOW;
            //case 3:
                //return GST_WEBRTC_PRIORITY_TYPE_MEDIUM;
            //case 4:
                //return GST_WEBRTC_PRIORITY_TYPE_HIGH;
            //default:
                //return GST_WEBRTC_PRIORITY_TYPE_VERY_LOW;
          //}
      //}

      //return GST_WEBRTC_PRIORITY_TYPE_VERY_LOW;
    //}


    //void
    //destroy_receiver_entry (gpointer receiver_entry_ptr)
    //{
        //ReceiverEntry *receiver_entry = (ReceiverEntry *) receiver_entry_ptr;

        //g_assert (receiver_entry != NULL);

        //gst_bin_remove (GST_BIN (pipeline), webrtc);
        //gst_object_unref (webrtc);

        //qname = g_strdup_printf ("queue-%s", peer_id);
        //q = gst_bin_get_by_name (GST_BIN (pipeline), qname);
        //g_free (qname);

        //sinkpad = gst_element_get_static_pad (q, "sink");
        //g_assert_nonnull (sinkpad);
        //srcpad = gst_pad_get_peer (sinkpad);
        //g_assert_nonnull (srcpad);
        //gst_object_unref (sinkpad);

        //gst_bin_remove (GST_BIN (pipeline), q);
        //gst_object_unref (q);

        //tee = gst_bin_get_by_name (GST_BIN (pipeline), "videotee");
        //g_assert_nonnull (tee);
        //gst_element_release_request_pad (tee, srcpad);
        //gst_object_unref (srcpad);
        //gst_object_unref (tee);

        //if (receiver_entry->webrtcbin != NULL) {
            //gst_object_unref (GST_OBJECT (receiver_entry->webrtcbin));
        //}

        //if (receiver_entry->connection != NULL) {
            //g_object_unref (G_OBJECT (receiver_entry->connection));
        //}

        //g_slice_free1 (sizeof (ReceiverEntry), receiver_entry);
    //}


    void
    on_offer_created_cb (GstPromise * promise, gpointer user_data)
    {
      gchar *sdp_string;
      gchar *json_string;
      JsonObject *sdp_json;
      JsonObject *sdp_data_json;
      GstStructure const *reply;
      GstPromise *local_desc_promise;
      GstWebRTCSessionDescription *offer = NULL;
      ReceiverEntry *receiver_entry = (ReceiverEntry *) user_data;

      reply = gst_promise_get_reply (promise);
      gst_structure_get (reply, "offer", GST_TYPE_WEBRTC_SESSION_DESCRIPTION,
          &offer, NULL);
      gst_promise_unref (promise);

      local_desc_promise = gst_promise_new ();
      g_signal_emit_by_name (receiver_entry->webrtcbin, "set-local-description",
          offer, local_desc_promise);
      gst_promise_interrupt (local_desc_promise);
      gst_promise_unref (local_desc_promise);

      sdp_string = gst_sdp_message_as_text (offer->sdp);
      gst_print ("Negotiation offer created:\n%s\n", sdp_string);

      sdp_json = json_object_new ();
      json_object_set_string_member (sdp_json, "type", "sdp");

      sdp_data_json = json_object_new ();
      json_object_set_string_member (sdp_data_json, "type", "offer");
      json_object_set_string_member (sdp_data_json, "sdp", sdp_string);
      json_object_set_object_member (sdp_json, "data", sdp_data_json);

      json_string = get_string_from_json_object (sdp_json);
      json_object_unref (sdp_json);

      soup_websocket_connection_send_text (receiver_entry->connection, json_string);
      g_free (json_string);
      g_free (sdp_string);

      gst_webrtc_session_description_free (offer);
    }


    void
    on_negotiation_needed_cb (GstElement * webrtcbin, gpointer user_data)
    {
      GstPromise *promise;
      ReceiverEntry *receiver_entry = (ReceiverEntry *) user_data;

      gst_print ("Creating negotiation offer\n");

      promise = gst_promise_new_with_change_func (on_offer_created_cb,
          (gpointer) receiver_entry, NULL);
      g_signal_emit_by_name (G_OBJECT (webrtcbin), "create-offer", NULL, promise);
    }


    void
    on_ice_candidate_cb (G_GNUC_UNUSED GstElement * webrtcbin, guint mline_index,
        gchar * candidate, gpointer user_data)
    {
      JsonObject *ice_json;
      JsonObject *ice_data_json;
      gchar *json_string;
      ReceiverEntry *receiver_entry = (ReceiverEntry *) user_data;

      ice_json = json_object_new ();
      json_object_set_string_member (ice_json, "type", "ice");

      ice_data_json = json_object_new ();
      json_object_set_int_member (ice_data_json, "sdpMLineIndex", mline_index);
      json_object_set_string_member (ice_data_json, "candidate", candidate);
      json_object_set_object_member (ice_json, "data", ice_data_json);

      json_string = get_string_from_json_object (ice_json);
      json_object_unref (ice_json);

      soup_websocket_connection_send_text (receiver_entry->connection, json_string);
      g_free (json_string);
    }


    void
    soup_websocket_message_cb (G_GNUC_UNUSED SoupWebsocketConnection * connection,
        SoupWebsocketDataType data_type, GBytes * message, gpointer user_data)
    {
      gsize size;
      gchar *data;
      gchar *data_string;
      const gchar *type_string;
      JsonNode *root_json;
      JsonObject *root_json_object;
      JsonObject *data_json_object;
      JsonParser *json_parser = NULL;
      ReceiverEntry *receiver_entry = (ReceiverEntry *) user_data;

      switch (data_type) {
        case SOUP_WEBSOCKET_DATA_BINARY:
          g_error ("Received unknown binary message, ignoring\n");
          g_bytes_unref (message);
          return;

        case SOUP_WEBSOCKET_DATA_TEXT:
          data = static_cast<gchar*>(g_bytes_unref_to_data (message, &size));
          /* Convert to NULL-terminated string */
          data_string = g_strndup (data, size);
          g_free (data);
          break;

        default:
          g_assert_not_reached ();
      }

      json_parser = json_parser_new ();
      if (!json_parser_load_from_data (json_parser, data_string, -1, NULL))
        goto unknown_message;

      root_json = json_parser_get_root (json_parser);
      if (!JSON_NODE_HOLDS_OBJECT (root_json))
        goto unknown_message;

      root_json_object = json_node_get_object (root_json);

      if (!json_object_has_member (root_json_object, "type")) {
        g_error ("Received message without type field\n");
        goto cleanup;
      }
      type_string = json_object_get_string_member (root_json_object, "type");

      if (!json_object_has_member (root_json_object, "data")) {
        g_error ("Received message without data field\n");
        goto cleanup;
      }
      data_json_object = json_object_get_object_member (root_json_object, "data");

      if (g_strcmp0 (type_string, "sdp") == 0) {
        const gchar *sdp_type_string;
        const gchar *sdp_string;
        GstPromise *promise;
        GstSDPMessage *sdp;
        GstWebRTCSessionDescription *answer;
        int ret;

        if (!json_object_has_member (data_json_object, "type")) {
          g_error ("Received SDP message without type field\n");
          goto cleanup;
        }
        sdp_type_string = json_object_get_string_member (data_json_object, "type");

        if (g_strcmp0 (sdp_type_string, "answer") != 0) {
          g_error ("Expected SDP message type \"answer\", got \"%s\"\n",
              sdp_type_string);
          goto cleanup;
        }

        if (!json_object_has_member (data_json_object, "sdp")) {
          g_error ("Received SDP message without SDP string\n");
          goto cleanup;
        }
        sdp_string = json_object_get_string_member (data_json_object, "sdp");

        gst_print ("Received SDP:\n%s\n", sdp_string);

        ret = gst_sdp_message_new (&sdp);
        g_assert_cmphex (ret, ==, GST_SDP_OK);

        ret =
            gst_sdp_message_parse_buffer ((guint8 *) sdp_string,
            strlen (sdp_string), sdp);
        if (ret != GST_SDP_OK) {
          g_error ("Could not parse SDP string\n");
          goto cleanup;
        }

        answer = gst_webrtc_session_description_new (GST_WEBRTC_SDP_TYPE_ANSWER,
            sdp);
        g_assert_nonnull (answer);

        promise = gst_promise_new ();
        g_signal_emit_by_name (receiver_entry->webrtcbin, "set-remote-description",
            answer, promise);
        gst_promise_interrupt (promise);
        gst_promise_unref (promise);
        gst_webrtc_session_description_free (answer);
      } else if (g_strcmp0 (type_string, "ice") == 0) {
        guint mline_index;
        const gchar *candidate_string;

        if (!json_object_has_member (data_json_object, "sdpMLineIndex")) {
          g_error ("Received ICE message without mline index\n");
          goto cleanup;
        }
        mline_index =
            json_object_get_int_member (data_json_object, "sdpMLineIndex");

        if (!json_object_has_member (data_json_object, "candidate")) {
          g_error ("Received ICE message without ICE candidate string\n");
          goto cleanup;
        }
        candidate_string = json_object_get_string_member (data_json_object,
            "candidate");

        gst_print ("Received ICE candidate with mline index %u; candidate: %s\n",
            mline_index, candidate_string);

        g_signal_emit_by_name (receiver_entry->webrtcbin, "add-ice-candidate",
            mline_index, candidate_string);
      } else
        goto unknown_message;

    cleanup:
      if (json_parser != NULL)
        g_object_unref (G_OBJECT (json_parser));
      g_free (data_string);
      return;

    unknown_message:
      g_error ("Unknown message \"%s\", ignoring", data_string);
      goto cleanup;
    }


    void
    soup_websocket_closed_cb (SoupWebsocketConnection * connection,
        gpointer user_data)
    {
      GHashTable *receiver_entry_table = (GHashTable *) user_data;
      g_hash_table_remove (receiver_entry_table, connection);
      gst_print ("Closed websocket connection %p\n", (gpointer) connection);
    }


    void
    soup_http_handler (G_GNUC_UNUSED SoupServer * soup_server,
        SoupMessage * message, const char *path, G_GNUC_UNUSED GHashTable * query,
        G_GNUC_UNUSED SoupClientContext * client_context,
        G_GNUC_UNUSED gpointer user_data)
    {
      SoupBuffer *soup_buffer;

      if ((g_strcmp0 (path, "/") != 0) && (g_strcmp0 (path, "/index.html") != 0)) {
        soup_message_set_status (message, SOUP_STATUS_NOT_FOUND);
        return;
      }

      soup_buffer =
          soup_buffer_new (SOUP_MEMORY_STATIC, html_source, strlen (html_source));

      soup_message_headers_set_content_type (message->response_headers, "text/html",
          NULL);
      soup_message_body_append_buffer (message->response_body, soup_buffer);
      soup_buffer_free (soup_buffer);

      soup_message_set_status (message, SOUP_STATUS_OK);
    }

    static gchar *
    get_string_from_json_object (JsonObject * object)
    {
      JsonNode *root;
      JsonGenerator *generator;
      gchar *text;

      /* Make it the root node */
      root = json_node_init_object (json_node_alloc (), object);
      generator = json_generator_new ();
      json_generator_set_root (generator, root);
      text = json_generator_to_data (generator, NULL);

      /* Release everything */
      g_object_unref (generator);
      json_node_free (root);
      return text;
    }


    //static GOptionEntry entries[] = {
      //{"video-priority", 0, 0, G_OPTION_ARG_STRING, &video_priority,
            //"Priority of the video stream (very-low, low, medium or high)",
          //"PRIORITY"},
      //{"audio-priority", 0, 0, G_OPTION_ARG_STRING, &audio_priority,
            //"Priority of the audio stream (very-low, low, medium or high)",
          //"PRIORITY"},
      //{NULL},
    //};

}

inline void testPadLink(unique_gobject_ptr<GstPad> src, unique_gobject_ptr<GstPad> sink) {
    auto ret = gst_pad_link(src.get(), sink.get());
    if(ret != GST_PAD_LINK_OK) {
        throw runtime_error("Erroneous padlink!");
    }
}

ReceiverEntry* create_receiver_entry(SoupWebsocketConnection* connection, GstElement* pipeline) {
    auto receiver_entry = static_cast<ReceiverEntry*>(g_slice_alloc0 (sizeof (ReceiverEntry)));
    receiver_entry->connection = connection;
    g_object_ref (G_OBJECT (connection));

    g_signal_connect (G_OBJECT (connection), "message",
        G_CALLBACK (soup_websocket_message_cb), (gpointer) receiver_entry);

    gst_print ("Attaching webrtcbin... (%p)\n", static_cast<void*>(receiver_entry));
    auto queueName = g_strdup_printf("queue-%p", static_cast<void*>(receiver_entry));
    auto q = gst_element_factory_make ("queue", queueName);
    g_free(queueName);

    receiver_entry->webrtcbin = gst_element_factory_make ("webrtcbin", NULL);

    gst_bin_add_many (GST_BIN (pipeline), q, receiver_entry->webrtcbin, NULL);

    testPadLink(
        make_unique_gobject(gst_element_get_static_pad(q, "src")),
        make_unique_gobject(gst_element_get_request_pad(receiver_entry->webrtcbin, "sink_%u"))
    );

    auto tee = make_unique_gobject(gst_bin_get_by_name(GST_BIN(pipeline), "videotee"));
    testPadLink(
        make_unique_gobject(gst_element_get_request_pad(tee.get(), "src_%u")),
        make_unique_gobject(gst_element_get_static_pad(q, "sink"))
    );

    cout << "Attached webrtcbin! (" << connection << ")" << endl;

    GArray *transceivers;
    g_signal_emit_by_name (receiver_entry->webrtcbin, "get-transceivers",
          &transceivers);
    g_assert (transceivers != NULL && transceivers->len > 0);
    auto trans = g_array_index (transceivers, GstWebRTCRTPTransceiver *, 0);
    g_object_set (trans, "direction",
        GST_WEBRTC_RTP_TRANSCEIVER_DIRECTION_SENDONLY, NULL);
    g_array_unref (transceivers);

    g_signal_connect (receiver_entry->webrtcbin, "on-negotiation-needed",
        G_CALLBACK (on_negotiation_needed_cb), (gpointer) receiver_entry);

    g_signal_connect (receiver_entry->webrtcbin, "on-ice-candidate",
        G_CALLBACK (on_ice_candidate_cb), (gpointer) receiver_entry);

    /* Set to pipeline branch to PLAYING */
    auto ret2 = gst_element_sync_state_with_parent (q);
    g_assert_true (ret2);
    ret2 = gst_element_sync_state_with_parent (receiver_entry->webrtcbin);
    g_assert_true (ret2);

    return receiver_entry;
}

void destroy_receiver_entry(gpointer receiver_entry_ptr) {
    ReceiverEntry* receiver_entry = static_cast<ReceiverEntry*>(receiver_entry_ptr);

    g_assert (receiver_entry != nullptr);

    gst_element_set_state(receiver_entry->webrtcbin, GST_STATE_NULL);
    gst_bin_remove(GST_BIN(pipeline.get()), receiver_entry->webrtcbin);
    gst_object_unref(receiver_entry->webrtcbin);

    auto qname = g_strdup_printf("queue-%p", static_cast<void*>(receiver_entry_ptr));
    auto q = make_unique_gobject(gst_bin_get_by_name(GST_BIN(pipeline.get()), qname));
    gst_element_set_state(q.get(), GST_STATE_NULL);
    g_free(qname);

    auto sinkpad = make_unique_gobject(gst_element_get_static_pad(q.get(), "sink"));
    auto srcpad = make_unique_gobject(gst_pad_get_peer(sinkpad.get()));

    gst_bin_remove(GST_BIN(pipeline.get()), q.get());

    auto tee = make_unique_gobject(gst_bin_get_by_name(GST_BIN(pipeline.get()), "videotee"));
    gst_element_release_request_pad(tee.get(), srcpad.get());

    if (receiver_entry->connection != NULL) {
        g_object_unref (G_OBJECT (receiver_entry->connection));
    }

    g_slice_free1 (sizeof (ReceiverEntry), receiver_entry);
}

void
soup_websocket_handler (G_GNUC_UNUSED SoupServer * server,
    SoupWebsocketConnection * connection, G_GNUC_UNUSED const char *path,
    G_GNUC_UNUSED SoupClientContext * client_context, gpointer user_data)
{
    ReceiverEntry *receiver_entry;
    GHashTable *receiver_entry_table = (GHashTable *) user_data;

    cout << "Processing new websocket connection " << connection << endl;

    g_signal_connect (G_OBJECT (connection), "closed",
        G_CALLBACK (soup_websocket_closed_cb), (gpointer) receiver_entry_table);

    receiver_entry = create_receiver_entry (connection, pipeline.get());
    g_hash_table_replace (receiver_entry_table, connection, receiver_entry);
}

int main (int argc, char** argv)
{
    Glib::init();
    gst_init(&argc, &argv);

    Glib::OptionGroup initialOptionGroup(gst_init_get_option_group());

    auto context = Glib::OptionContext("Open monitor");
    context.add_group(initialOptionGroup);

    try {
       if(! context.parse(argc, argv)) {
            cerr << "Error parsing command line arguments!" << endl;
            return EXIT_FAILURE;
       }
    } catch(const Glib::OptionError& e) {
        cerr << "Error parsing command line option: " << e.what();
        return EXIT_FAILURE;
    } catch(const Glib::ConvertError& e) {
        cerr << "Error parsing command line option: " << e.what();
        return EXIT_FAILURE;
    }

    auto receiver_entry_table = unique_ptr<GHashTable, std::function<void(GHashTable*)>>(
        g_hash_table_new_full(g_direct_hash, g_direct_equal, nullptr, destroy_receiver_entry),
        [](auto table) {
            g_hash_table_destroy (table);
        }
    );

    auto mainLoop = Glib::MainLoop::create(false);

    GError* error = nullptr;
    pipeline = unique_ptr<GstElement, std::function<void(GstElement*)>>(
          gst_parse_launch ("tee name=videotee ! queue ! fakesink "
          "rpicamsrc exposure-mode=night annotation-mode=512 ! video/x-h264,width=1920,height=1080,framerate=3/1 ! queue ! h264parse ! "
          "rtph264pay config-interval=-1 name=payloader aggregate-mode=zero-latency ! "
          "application/x-rtp,media=video,encoding-name=H264,payload=96 ! videotee. "
    /*      "autoaudiosrc is-live=1 ! queue max-size-buffers=1 leaky=downstream ! audioconvert ! audioresample ! opusenc ! rtpopuspay pt="
          RTP_AUDIO_PAYLOAD_TYPE " ! webrtcbin. "*/, &error),
          [](auto pipeline) {
                gst_element_set_state (GST_ELEMENT(pipeline), GST_STATE_NULL);
                gst_object_unref(GST_OBJECT(pipeline));
          }
    );

    if (error != nullptr) {
        cerr << "Could not create WebRTC pipeline: " << error->message << endl;
        g_error_free (error);
        return EXIT_FAILURE;
    }

    auto bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline.get()));
    gst_bus_add_watch (bus, bus_watch_cb, NULL);
    gst_object_unref (bus);

    if (gst_element_set_state(pipeline.get(), GST_STATE_PLAYING) == GST_STATE_CHANGE_FAILURE) {
        cerr << "Could not start pipeline" << endl;
        return EXIT_FAILURE;
    }

    auto soup_server = unique_ptr<SoupServer, std::function<void(SoupServer*)>>(
        soup_server_new (SOUP_SERVER_SERVER_HEADER, "open-monitor-soup-server", nullptr),
        [](auto server) {
            g_object_unref(G_OBJECT(server));
        }
    );
    soup_server_add_handler(soup_server.get(), "/", soup_http_handler, NULL, NULL);
    soup_server_add_websocket_handler(soup_server.get(), "/ws", nullptr, nullptr, soup_websocket_handler, receiver_entry_table.get(), nullptr);
    soup_server_listen_all(soup_server.get(), soupHttpPort, SoupServerListenOptions(), nullptr);

    cout << "WebRTC page link: http://127.0.0.1:" << soupHttpPort << endl;

    mainLoop->run();

    cout << "Stopped mainloop" << endl;

    gst_deinit ();

    return EXIT_SUCCESS;
}
