
#ifndef UTILITY_TRACE_H_
#define UTILITY_TRACE_H_

#include <string>
using std::string;
#include <vector>
using std::vector;

namespace UTILITY {

    /* trace functionality:
     * > object based trace output
     * > classified trace output, with control of enable/disable classify rule
     * > auto dump all trace info of current thread, in case of error (exception throw out), to provide debug clue
     * > dump trace at the construction/destruction of trace object, used for function entrance/exit tracing
     */

    namespace TRC {

        void printf(const char * fmt, ...)
             __attribute__ ((format (printf, 1, 2)));

    }

    // error(exception) trigger trace output of all trace objs in current thread

    class TRACE_BASE {
    public:
        TRACE_BASE();
        virtual ~TRACE_BASE();

        virtual void dump(void)  = 0;
        static  void dump_all(void);

        struct INFO;
    protected:
        void on_destroy();
        static void dump_all(INFO &);

        INFO &info;
    };

    class TRACE_P0 : public TRACE_BASE {
    public:
        TRACE_P0(const char * format)
            : fmt(format)
            { }

            ~TRACE_P0()
            { on_destroy(); }

            void dump(void)
            { TRC::printf(fmt); }

        private:
            const char * fmt;
    };

    template<typename T1>
    class TRACE_P1 : public TRACE_BASE {
    public:
        TRACE_P1(const char * format, const T1 &arg_1)
        : fmt(format),p1(arg_1)
        { }

        ~TRACE_P1()
        { on_destroy(); }

        void dump(void)
        { TRC::printf(fmt, p1); }

    private:
        const char * fmt;
        const T1 &p1;
    };

    template<typename T1, typename T2>
    class TRACE_P2 : public TRACE_BASE {
    public:
        TRACE_P2(const char *format, const T1 &arg_1, const T2 &arg_2)
        : fmt(format),p1(arg_1),p2(arg_2)
        { }

        ~TRACE_P2()
        { on_destroy(); }

        void dump(void)
        { TRC::printf(fmt, p1, p2); }

    private:
        const char * fmt;
        const T1 &p1;
        const T2 &p2;
    };

    template<typename T1, typename T2, typename T3>
    class TRACE_P3 : public TRACE_BASE {
    public:
        TRACE_P3(const char *format, const T1 &arg_1, const T2 &arg_2, const T3 &arg_3)
        : fmt(format),p1(arg_1),p2(arg_2),p3(arg_3)
        { }

        ~TRACE_P3()
        { on_destroy(); }

        void dump(void)
        { TRC::printf(fmt, p1, p2, p3); }

    private:
        const char * fmt;
        const T1 &p1;
        const T2 &p2;
        const T3 &p3;
    };

    template<typename T1, typename T2, typename T3, typename T4>
    class TRACE_P4 : public TRACE_BASE {
    public:
        TRACE_P4(const char *format, const T1 &arg_1, const T2 &arg_2, const T3 &arg_3, const T4 &arg_4)
        : fmt(format),p1(arg_1),p2(arg_2),p3(arg_3),p4(arg_4)
        { }

        ~TRACE_P4()
        { on_destroy(); }

        void dump(void)
        { TRC::printf(fmt, p1, p2, p3, p4); }

    private:
        const char * fmt;
        const T1 &p1;
        const T2 &p2;
        const T3 &p3;
        const T4 &p4;
    };

    template<typename T1, typename T2, typename T3, typename T4, typename T5>
    class TRACE_P5 : public TRACE_BASE {
    public:
        TRACE_P5(const char *format, const T1 &arg_1, const T2 &arg_2, const T3 &arg_3, const T4 &arg_4, const T5 &arg_5)
        : fmt(format),p1(arg_1),p2(arg_2),p3(arg_3),p4(arg_4),p5(arg_5)
        { }

        ~TRACE_P5()
        { on_destroy(); }

        void dump(void)
        { TRC::printf(fmt, p1, p2, p3, p4, p5); }

    private:
        const char * fmt;
        const T1 &p1;
        const T2 &p2;
        const T3 &p3;
        const T4 &p4;
        const T5 &p5;
    };

    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
    class TRACE_P6 : public TRACE_BASE {
    public:
        TRACE_P6(const char *format, const T1 &arg_1, const T2 &arg_2, const T3 &arg_3, const T4 &arg_4, const T5 &arg_5, const T6 &arg_6)
        : fmt(format),p1(arg_1),p2(arg_2),p3(arg_3),p4(arg_4),p5(arg_5),p6(arg_6)
        { }

        ~TRACE_P6()
        { on_destroy(); }

        void dump(void)
        { TRC::printf(fmt, p1, p2, p3, p4, p5, p6); }

    private:
        const char * fmt;
        const T1 &p1;
        const T2 &p2;
        const T3 &p3;
        const T4 &p4;
        const T5 &p5;
        const T6 &p6;
    };
    //run time trace output, with trace classified by trace filter
    //static instance of filter is allowed
    class TRACE_FILTER {
    public:
        TRACE_FILTER(const char *name);
        ~TRACE_FILTER();

        void enable ();
        void disable();

        static void enable (const char *name);
        static void disable(const char *name);

        static vector<string> get_disabled();
        static vector<string> get_enabled();

        bool is_enabled() const;
        static bool is_enabled(const char *name);

    private:
        const string name;
        bool m_is_enabled;
    };

    TRACE_P0
    make_trace(const TRACE_FILTER &filter, const char *fmt);

    template<typename T1>
    TRACE_P1<T1>
    make_trace(const TRACE_FILTER &filter, const char *fmt, const T1 &p1)
    {
        TRACE_P1<T1> trc(fmt, p1);
        if(filter.is_enabled()) trc.dump();
        return trc;
    }

    template<typename T1, typename T2>
    TRACE_P2<T1,T2>
    make_trace(const TRACE_FILTER &filter, const char *fmt, const T1 &p1, const T2 &p2)
    {
        TRACE_P2<T1,T2> trc(fmt, p1, p2);
        if(filter.is_enabled()) trc.dump();
        return trc;
    }

    template<typename T1, typename T2, typename T3>
    TRACE_P3<T1,T2,T3>
    make_trace(const TRACE_FILTER &filter, const char *fmt, const T1 &p1, const T2 &p2, const T3 &p3)
    {
        TRACE_P3<T1,T2,T3> trc(fmt, p1, p2, p3);
        if(filter.is_enabled()) trc.dump();
        return trc;
    }

    template<typename T1, typename T2, typename T3, typename T4>
    TRACE_P4<T1,T2,T3,T4>
    make_trace(const TRACE_FILTER &filter, const char *fmt, const T1 &p1, const T2 &p2, const T3 &p3, const T4 &p4)
    {
        TRACE_P4<T1,T2,T3,T4> trc(fmt, p1, p2, p3, p4);
        if(filter.is_enabled()) trc.dump();
        return trc;
    }

    template<typename T1, typename T2, typename T3, typename T4, typename T5>
    TRACE_P5<T1,T2,T3,T4,T5>
    make_trace(const TRACE_FILTER &filter, const char *fmt, const T1 &p1, const T2 &p2, const T3 &p3, const T4 &p4, const T5 &p5)
    {
        TRACE_P5<T1,T2,T3,T4,T5> trc(fmt, p1, p2, p3, p4, p5);
        if(filter.is_enabled()) trc.dump();
        return trc;
    }

    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
    TRACE_P6<T1,T2,T3,T4,T5,T6>
    make_trace(const TRACE_FILTER &filter, const char *fmt, const T1 &p1, const T2 &p2, const T3 &p3, const T4 &p4, const T5 &p5, const T6 &p6)
    {
        TRACE_P6<T1,T2,T3,T4,T5,T6> trc(fmt, p1, p2, p3, p4, p5,p6);
        if(filter.is_enabled()) trc.dump();
        return trc;
    }

    //construction/destruction trigger trace output

    class DURATION_TRACE : public TRACE_BASE {
    public:
        DURATION_TRACE(const TRACE_FILTER &_filter, const char *_str);
        ~DURATION_TRACE();

        void dump(void);

    private:
        const TRACE_FILTER &filter;
        const char * str;
    };

}

//////////////// helper /////////////////

#define LINENAME_CAT(name, line) name##line
#define LINENAME(name, line) LINENAME_CAT(name, line)

#define _TRACE(filter,args...) \
        const UTILITY::TRACE_BASE & LINENAME(_trc_,__LINE__) = UTILITY::make_trace(filter,args)

//example of classified trace
//#define TRC_SUB(filter,args...) _TRACE(_filter_sub, args)

//example of auto print the enter and left of a function
//#define AUTO_TRC_FUNC DURATION_TRACE _auto_trc_func(_filter_func,__FUNCTION__)

#endif /* UTILITY_TRACE_H_ */
