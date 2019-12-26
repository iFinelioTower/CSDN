需要修改两个部分
1. AudioFlinger (framworks/av/service/audioflinger)
        modified:   services/audioflinger/AudioFlinger.cpp
        modified:   services/audioflinger/AudioFlinger.h
        modified:   services/audioflinger/AudioMixer.h
        modified:   services/audioflinger/RecordTracks.h
        modified:   services/audioflinger/Threads.cpp
        modified:   services/audioflinger/Threads.h
        modified:   services/audioflinger/TrackBase.h
        modified:   services/audioflinger/Tracks.cpp

2. AudioPolicy (hardware/rk29/audio)
        modified:   include/hardware_legacy/AudioPolicyManagerBase.h

3. 若干头文件. (system/*)

您可以先比较旧代码(Origin_AudioFrmaeWork):
    如果一致, 则直接将适配代码替换.
    如果不一致, 把差异点比对过去.


Android5.1 的调用过程, 其中与 Andorid4.4 最大的差异在于 mActiveTracks 的设计.
set
    openRecord
        AudioSystem::getInput
            // 只有 openInput 才回去创建新的 RecordThread.
            // 这里做了处理, 当本次录音的 Profile 文件与已经打开的 
            // mInput Profile 一样的话, 表示希望打开同一设备. 仅返回 audio_io_handle_t 即可.           
            mpClientInterface->openInput
                AudioFlinger::openInput
                    // 创建一个新的 audio_io_handle_t 号码.
                    audio_io_handle_t id = nextUniqueId();
                    
                    // 建立与 so 库的连接, 注意: 此时还没有 open 底层驱动.
                    inHwHal->open_input_stream(inHwHal, id, *pDevices, &config, &inStream);
                        ladev->hwif->openInputStream
                            // 创建 speex_resampler_init 重采样器.
                            new AudioHardware::DownSampler 
                            
                    // 创建一个 RecordThread 录音线程
                    thread = new RecordThread();
                    
                    // 将 audio_io_handle_t 绑定 RecordThread.
                    mRecordThreads.add(id, thread);
                    
        record = audioFlinger->openRecord
            // 从 mRecordThreads 里面根据 input(id) 获取一个 RecordThread.
            thread = checkRecordThread_l(input);
            // 创建一个 RecordTrack. 
            recordTrack = thread->createRecordTrack_l();
                new RecordTrack()
                
            // 将 recordTrack 绑定 RecordHandle 
            recordHandle = new RecordHandle(recordTrack);
            return recordHandle;
            
        mAudioRecord = record;
        
start
    mAudioRecord->start(event, triggerSession);
        RecordHandle->start()
            RecordThread::start(recordTrack)
                // 在此处加入 mActiveTracks 数组
                mActiveTracks.add(recordTrack); 
                
                // 置位 recordTrack != TrackBase::IDLE 状态.
                recordTrack->mState = TrackBase::STARTING_2;
                
                // 释放 RecordThread::threadLoop() 的锁.
                mWaitWorkCV.broadcast();
                
                
// 停止只是将 recordTrack = TrackBase::PAUSING 的状态
// 不在 memcpy 底层获得到的 buffer 而已.
stop 
    mRecordTrack->stop();
        AudioSystem::stopInput(recordThread->id());

        
// 当 RecordHandle 析构时, 也就是持有它的 AudioRecord 对象析构时才调用.
~RecordHandle
    RecordTrack::destroy
        AudioSystem::releaseInput
            mpClientInterface->closeInput 
                AudioFlinger::closeInput
                    thread = checkRecordThread_l(input);
                    mRecordThreads.removeItem(input);
                    in->hwDev()->close_input_stream(in->hwDev(), in->stream);
    
        recordThread->destroyTrack_l
            // 仅仅是指移除了在 mTracks 里面的 recordTrack. 没有移除 mActiveTracks 里面的 recordTrack.
            mTracks.remove(track);