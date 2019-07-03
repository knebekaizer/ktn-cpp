//package sample.spdlog

import kotlinx.cinterop.*
import spdlog.*

fun main() {
    val name = "You"
    println("Hey $name")
    test()
}

fun test() {
	var log = log_create();
/*
	if (!log) {
		fprintf(stderr, "Logger initialization error\n");
		exit(100);
	}
*/
	log_write(log, "Using external logger");

}

/*
object git {
    init {
        git_libgit2_init()
    }

    fun close() {
        git_libgit2_shutdown()
    }

    fun repository(location: String): GitRepository {
        return GitRepository(location)
    }
}

fun Int.errorCheck() {
    if (this == 0) return
    throw GitException()
}

class GitException : Exception(run {
    val err = giterr_last()
    err!!.pointed.message!!.toKString()
})
*/
