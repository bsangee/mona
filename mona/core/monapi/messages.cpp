#include <monapi/syscall.h>
#include <monapi/messages.h>
#include <monapi/SharedMemory.h>
#include <monapi/Message.h>
#include <monapi/Stream.h>
#include <monapi/System.h>
#include <monapi/string.h>
#include <sys/HashMap.h>
#include <monapi.h>

using namespace MonAPI;

static uint32_t name_server_id = THREAD_UNKNOWN;
static HashMap<uint32_t> nameCache(64);

static uint32_t server_ids[] =
{
    THREAD_UNKNOWN,  // ID_MOUSE_SERVER
    THREAD_UNKNOWN,  // ID_KEYBOARD_SERVER
    THREAD_UNKNOWN,  // ID_FILE_SERVER
    THREAD_UNKNOWN,  // ID_GUI_SERVER
    THREAD_UNKNOWN,  // ID_ELF_SERVER
    THREAD_UNKNOWN,  // ID_PROCESS_SERVER
    THREAD_UNKNOWN,  // ID_PE_SERVER
    THREAD_UNKNOWN,  // ID_MONITOR_SERVER
    THREAD_UNKNOWN,  // ID_SCHEME_SERVER
    THREAD_UNKNOWN,  // ID_NET_SERVER
    THREAD_UNKNOWN   // ID_CLIPBOARD_SERVER
};

static const char* server_names[] =
{
    "MOUSE.EX5", "KEYBDMNG.EX5", "FILE.BIN", "GUI.EX5", "ELF.BN5", "PROCESS.BIN", "PE.BN5", "MONITOR.BIN", "SCHEME.EX5", "NET.EX5", "CLIPBRD.EX5"
};

intptr_t monapi_name_whereis(const char* name, uint32_t& id)
{
    uint32_t found = nameCache.get(name);
    if (found != 0) {
        id = found;
        return M_OK;
    }
    uint32_t name_server;
    intptr_t ret = monapi_name_get_server(name_server);
    if (ret != M_OK) {
        monapi_warn("name server not found");
        return ret;
    }
    MessageInfo dest;
    ret = Message::sendReceive(&dest, name_server, MSG_WHERE, 0, 0, 0, name);
    if (ret != M_OK) {
        monapi_warn("name server returns error");
        return ret;
    }
    if (dest.arg2 != M_OK) {
        monapi_warn("name %s not found", name);
        return dest.arg2;
    }
    id = dest.arg3;
    nameCache.put(name, id);
    return M_OK;
}

intptr_t monapi_name_add(const char* name)
{
    uint32_t name_server;
    intptr_t ret = monapi_name_get_server(name_server);
    if (ret != M_OK) {
        monapi_warn("name server not found :%s", monapi_error_string(ret));
        return ret;
    }
    ASSERT(strlen(name) < MESSAGE_INFO_MAX_STR_LENGTH);
    ret =  Message::sendReceive(NULL, name_server, MSG_ADD, 0, 0, 0, name);
    return ret;
}

intptr_t monapi_name_get_server(uint32_t& id)
{
    if (name_server_id != THREAD_UNKNOWN) {
        id = name_server_id;
        return M_OK;
    }
    intptr_t ret = Message::sendAll(MSG_NAME);
    if (ret != M_OK) {
        monapi_warn("MSG_NAME broadcast failed : %s", monapi_error_string(ret));
        return ret;
    }
    MessageInfo src, dest;
    src.header = MSG_OK;
    src.arg1 = MSG_NAME;
    ret = Message::receive(&dest, &src, Message::equalsHeaderArg1);
    if (ret != M_OK) {
        monapi_warn("MSG_NAME receive failed : %s", monapi_error_string(ret));
        return ret;
    }
    id = dest.from;
    name_server_id = id;
    return M_OK;
}

uint32_t monapi_get_server_thread_id(int id)
{
    if (id < 0 || ID_NUMBER_OF_SERVERS <= id) return THREAD_UNKNOWN;

    if (server_ids[id] == THREAD_UNKNOWN)
    {
        server_ids[id] = Message::lookupMainThread(server_names[id]);
        if (server_ids[id] == THREAD_UNKNOWN)
        {
           MONAPI_WARN("ERROR: can not connect to %s", server_names[id]);
        }

        // quick hack.
        if (id == ID_NET_SERVER) {
            server_ids[id] = server_ids[id] + 2;
        }
    }
    return server_ids[id];
}

intptr_t monapi_register_to_server(int id)
{
    uint32_t tid = monapi_get_server_thread_id(id);
    if (tid == THREAD_UNKNOWN) return M_NAME_NOT_FOUND;

    if (Message::sendReceive(NULL, tid, MSG_ADD, syscall_get_tid()) != M_OK)
    {
        MONAPI_WARN("ERROR: can not register to %s", server_names[id]);
        return M_UNKNOWN;
    }
    return M_OK;
}

intptr_t monapi_unregister_to_server(int id)
{
    uint32_t tid = monapi_get_server_thread_id(id);
    if (tid == THREAD_UNKNOWN) return M_NAME_NOT_FOUND;

    if (Message::sendReceive(NULL, tid, MSG_REMOVE, syscall_get_tid()) != M_OK)
    {
        MONAPI_WARN("ERROR: can not register to %s", server_names[id]);
        return M_UNKNOWN;
    }
    return M_OK;
}

MONAPI_BOOL monapi_call_mouse_set_cursor(MONAPI_BOOL enabled)
{
    uint32_t tid ;
    if (monapi_name_whereis("/servers/mouse", tid) != M_OK) {
        return MONAPI_FALSE;
    }
    uint32_t header = enabled ? MSG_MOUSE_ENABLE_CURSOR : MSG_MOUSE_DISABLE_CURSOR;
    MessageInfo msg;
    if (Message::sendReceive(&msg, tid, header) != M_OK)
    {
        return MONAPI_FALSE;
    }
    return MONAPI_TRUE;
}


SharedMemory* monapi_call_file_decompress_bz2_file(const char* file, MONAPI_BOOL prompt)
{
    uint32_t tid;
    if (monapi_name_whereis("/servers/file", tid) != M_OK) {
        return NULL;
    }

    MessageInfo msg;
    if (Message::sendReceive(&msg, tid, MSG_FILE_DECOMPRESS_BZ2_FILE, prompt, 0, 0, file) != M_OK)
    {
        return NULL;
    }

    if (msg.arg2 == 0) return NULL;
    SharedMemory* shm = new SharedMemory(msg.arg2, msg.arg3);
    if (shm->map(true) != M_OK) {
        delete shm;
        return NULL;
    } else {
        return shm;
    }
}

SharedMemory* monapi_call_file_decompress_bz2(const SharedMemory& shm)
{
    uint32_t tid;
    if (monapi_name_whereis("/servers/file", tid) != M_OK) {
        return NULL;
    }
    MessageInfo msg;
    if (Message::sendReceive(&msg, tid, MSG_FILE_DECOMPRESS_BZ2, shm.handle(), shm.size()) != M_OK) {
        return NULL;
    }
    if (msg.arg2 == 0) return NULL;

    SharedMemory* ret = new SharedMemory(msg.arg2, msg.arg3);
    if (ret->map(true) != M_OK) {
        delete ret;
        return NULL;
    } else {
        return ret;
    }
}


SharedMemory* monapi_call_file_decompress_st5(const SharedMemory& shm)
{
    uint32_t tid;
    if (monapi_name_whereis("/servers/file", tid) != M_OK) {
        return NULL;
    }
    MessageInfo msg;
    if (Message::sendReceive(&msg, tid, MSG_FILE_DECOMPRESS_ST5, shm.handle(), shm.size()) != M_OK)
    {
        return NULL;
    }
    if (msg.arg2 == 0) return NULL;

    SharedMemory* ret = new SharedMemory(msg.arg2, msg.arg3);
    if (ret->map(true) != M_OK) {
        delete ret;
        return NULL;
    } else {
        return ret;
    }
}

SharedMemory* monapi_call_file_decompress_st5_file(const char* file, MONAPI_BOOL prompt)
{
    uint32_t tid;
    if (monapi_name_whereis("/servers/file", tid) != M_OK) {
        return NULL;
    }

    MessageInfo msg;
    if (Message::sendReceive(&msg, tid, MSG_FILE_DECOMPRESS_ST5_FILE, prompt, 0, 0, file) != M_OK)
    {
        return NULL;
    }

    if (msg.arg2 == 0) {
        int status = Message::reply(&msg);
        if (status != M_OK) {
            monapi_warn("%s reply failed : %s\n", __func__, monapi_error_string(status));
        }
        return NULL;
    }
    SharedMemory* shm = new SharedMemory(msg.arg2, msg.arg3);
    if (shm->map(true) != M_OK) {
        delete shm;
        int status = Message::reply(&msg);
        if (status != M_OK) {
            monapi_warn("%s reply failed : %s\n", __func__, monapi_error_string(status));
        }
        return NULL;
    } else {
        int status = Message::reply(&msg);
        if (status != M_OK) {
            monapi_warn("%s reply failed : %s\n", __func__, monapi_error_string(status));
        }
        return shm;
    }
}

intptr_t monapi_call_process_execute_file(const char* command_line, MONAPI_BOOL prompt)
{
    return monapi_call_process_execute_file_get_tid(command_line, prompt, NULL, NULL, NULL);
}

intptr_t monapi_call_process_execute_file_get_tid(const char* command_line, MONAPI_BOOL prompt, uint32_t* tid, uint32_t stdin_id /* = NULL */, uint32_t stdout_id /* NULL */)
{
    uint32_t server;
    if (monapi_name_whereis("/servers/process", server) != M_OK) {
        return M_NAME_NOT_FOUND;
    }
    MessageInfo msg;
    if (Message::sendReceive(&msg, server, MSG_PROCESS_EXECUTE_FILE, prompt, stdin_id, stdout_id, command_line) != M_OK)
    {
        if (tid != NULL) *tid = THREAD_UNKNOWN;
        return -1;
    }
    if (tid != NULL) *tid = msg.arg3;
    return msg.arg2;
}

// size should be 4096 * x
uint8_t* monapi_allocate_dma_memory(int size)
{
    return syscall_allocate_dma_memory(size);
}

// size should be 4096 * x
void monapi_deallocate_dma_memory(void* address, int size)
{
    syscall_deallocate_dma_memory(address, size);
}


intptr_t monapi_file_open(const char* file, intptr_t mode)
{
    uint32_t tid;
    if (monapi_name_whereis("/servers/file", tid) != M_OK) {
        return M_NAME_NOT_FOUND;
    }
    MessageInfo msg;
    int ret = Message::sendReceive(&msg, tid, MSG_FILE_OPEN, mode, 0, 0, file);
    if (ret != M_OK) {
        return ret;
    }
    return msg.arg2;
}

static SharedMemory* file_receive_shm(int header, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3, const char* str)
{
    uint32_t tid = monapi_get_server_thread_id(ID_FILE_SERVER);
    MessageInfo msg;
    if (Message::sendReceive(&msg, tid, header, arg1, arg2, arg3, str) != M_OK) {
        return NULL;
    }
    if ((intptr_t)msg.arg2 < M_OK) {
        int status = Message::reply(&msg);
        if (status != M_OK) {
            monapi_warn("%s reply failed : %s\n", __func__, monapi_error_string(status));
        }
        return NULL;
    }
    uintptr_t handle = msg.arg2;
    uintptr_t size = msg.arg3;
    SharedMemory* shm = new SharedMemory(handle, size);
    if (shm->map(true) != M_OK) {
        int status = Message::reply(&msg);
        delete shm;
        if (status != M_OK) {
            monapi_warn("%s reply failed : %s\n", __func__, monapi_error_string(status));
        }
        return NULL;
    } else {
        int status = Message::reply(&msg);
        if (status != M_OK) {
            monapi_warn("%s reply failed : %s\n", __func__, monapi_error_string(status));
        }
        return shm;
    }
}

SharedMemory* monapi_file_read_all(const char* file)
{
    return file_receive_shm(MSG_FILE_READ_ALL, 0, 0, 0, file);
}

SharedMemory* monapi_file_read(uint32_t fileID, uint32_t size)
{
    return file_receive_shm(MSG_FILE_READ, fileID, size, 0, NULL);
}

SharedMemory* monapi_file_read_directory(const char* path)
{
    return file_receive_shm(MSG_FILE_READ_DIRECTORY, 0, 0, 0, path);
}

intptr_t monapi_file_seek(uint32_t fileID, int32_t offset, uint32_t origin)
{
    uint32_t tid;
    if (monapi_name_whereis("/servers/file", tid) != M_OK) {
        return M_NAME_NOT_FOUND;
    }
    MessageInfo msg;
    if (Message::sendReceive(&msg, tid, MSG_FILE_SEEK, fileID, offset, origin) != M_OK)
    {
        return M_UNKNOWN;
    }
    return msg.arg2;
}

intptr_t monapi_file_close(uint32_t fileID)
{
    uint32_t tid;
    if (monapi_name_whereis("/servers/file", tid) != M_OK) {
        return M_NAME_NOT_FOUND;
    }
    MessageInfo msg;
    int ret = Message::sendReceive(&msg, tid, MSG_FILE_CLOSE, fileID);
    if (ret != M_OK)
    {
        return ret;
    }
    return msg.arg2;
}

intptr_t monapi_file_get_file_size(uint32_t id)
{
    uint32_t tid;
    if (monapi_name_whereis("/servers/file", tid) != M_OK) {
        return M_NAME_NOT_FOUND;
    }

    MessageInfo msg;
    intptr_t ret = Message::sendReceive(&msg, tid, MSG_FILE_GET_SIZE, id);
    if (ret != M_OK)
    {
        return ret;
    }
    return msg.arg3;
}

intptr_t monapi_file_delete(const char* file)
{
    uint32_t tid;
    if (monapi_name_whereis("/servers/file", tid) != M_OK) {
        return M_NAME_NOT_FOUND;
    }
    MessageInfo msg;
    intptr_t ret = Message::sendReceive(&msg, tid, MSG_FILE_DELETE, 0, 0, 0, file);
    if (ret != M_OK) {
        return ret;
    }
    return msg.arg2;
}


intptr_t monapi_file_stop_server()
{
    uint32_t tid;
    if (monapi_name_whereis("/servers/file", tid) != M_OK) {
        return M_NAME_NOT_FOUND;
    }
    MessageInfo msg;
    int ret = Message::sendReceive(&msg, tid, MSG_STOP_SERVER);
    if (ret != M_OK) {
        return ret;
    }
    return msg.arg2;
}

intptr_t monapi_file_write(uint32_t fileID, const SharedMemory& mem, uint32_t size)
{
    MessageInfo msg;
    uint32_t tid;
    if (monapi_name_whereis("/servers/file", tid) != M_OK) {
        return M_NAME_NOT_FOUND;
    }
    int ret = Message::sendReceive(&msg, tid, MSG_FILE_WRITE, fileID, size, mem.handle());
    if (ret != M_OK) {
        return ret;
    }
    return msg.arg2;
}

extern Stream* inStream;
extern Stream* outStream;

uint32_t monapi_stdin_read(uint8_t* buffer, uint32_t size)
{
    System::getStdinStream();
    inStream->waitForRead();
    return inStream->read(buffer, size);
}

intptr_t monapi_stdout_write(uint8_t* buffer, uint32_t size)
{
    System::getStdoutStream();
    if (NULL == outStream || outStream->isInvalid())
    {
        monapi_warn("You can't use printf, use _printf instead.");
        monapi_warn("outStream=%x isInvalid=%d", outStream, outStream ? outStream->isInvalid() : 1);
        return -1;
    }
    return outStream->write(buffer, size);
}

uint32_t monapi_stdin_lock_for_read()
{
    System::getStdinStream();
    return inStream->lockForRead();
}

uint32_t monapi_stdin_try_lock_for_read()
{
    System::getStdinStream();
    return inStream->tryLockForRead();
}

uint32_t monapi_stdin_unlock_for_read()
{
    System::getStdinStream();
    return inStream->unlockForRead();
}

intptr_t monapi_process_wait_terminated(uint32_t tid)
{
    for (MessageInfo msg;;)
    {
        if (MonAPI::Message::receive(&msg) != 0) continue;
        switch (msg.header)
        {
        case MSG_PROCESS_TERMINATED:
            if (tid == msg.arg1)
            {
                // exit status code
                return msg.arg2;
            }
            break;
        default:
            break;
        }
    }
    return M_UNKNOWN;
}

intptr_t monapi_notify_server_start(const char* name)
{
    uint32_t targetID = Message::lookupMainThread(name);

    if (targetID == THREAD_UNKNOWN)
    {
        MONAPI_WARN("%s:INIT not found", name);
        return M_NAME_NOT_FOUND;
    }

    /* send */
    if(Message::send(targetID, MSG_STARTED) != M_OK)
    {
        MONAPI_WARN("%s:INIT error", name);
        return M_NAME_NOT_FOUND;
    }
    return M_OK;
}

MONAPI_BOOL monapi_file_exists(const char* path)
{
    intptr_t desc = monapi_file_open(path, 0);
    if (desc > 0) {
        monapi_file_close(desc);
        return MONAPI_TRUE;
    } else {
        scoped_ptr<SharedMemory> shm(monapi_file_read_directory(path));
        if (shm.get() == NULL) {
            return MONAPI_FALSE;
        } else {
            return MONAPI_TRUE;
        }
    }
}

intptr_t monapi_clipboard_set(const SharedMemory& shm)
{
    MessageInfo msg;
    uint32_t tid ;
    if (monapi_name_whereis("/servers/clipboard", tid) != M_OK) {
        monapi_warn("");
        return M_NAME_NOT_FOUND;
    }
        monapi_warn("");
    intptr_t ret = Message::sendReceive(&msg, tid, MSG_CLIPBOARD_SET, shm.handle(), shm.size());
    if (ret != M_OK) {
        monapi_warn("");
        return ret;
    }
        monapi_warn("");
    return M_OK;
}

intptr_t monapi_clipboard_clear()
{
    MessageInfo msg;
    uint32_t tid ;
    if (monapi_name_whereis("/servers/clipboard", tid) != M_OK) {
        return M_NAME_NOT_FOUND;
    }
    intptr_t ret = Message::sendReceive(&msg, tid, MSG_CLIPBOARD_CLEAR);
    if (ret != M_OK) {
        return ret;
    }
    return M_OK;
}

SharedMemory* monapi_clipboard_get()
{
    uint32_t tid ;
    if (monapi_name_whereis("/servers/clipboard", tid) != M_OK) {
        return NULL;
    }

    MessageInfo msg;
    if (Message::sendReceive(&msg, tid, MSG_CLIPBOARD_GET) != M_OK) {
        return NULL;
    }
    if ((intptr_t)msg.arg2 < M_OK) { return NULL;}
    SharedMemory* ret = new SharedMemory(msg.arg2, msg.arg3);
    if (ret->size() == 0) {
        return ret;
    }
    intptr_t mapResult = ret->map(true);
    if (mapResult != M_OK) {
        delete ret;
        monapi_warn("%s map error = %d\n", __func__, mapResult);
        return NULL;
    } else {
        return ret;
    }
}
